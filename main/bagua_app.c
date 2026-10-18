// main/bagua_app.c —— 八卦罗盘应用生命周期、按键调度与起卦动画。
//
// 交互约定（自定义，不沿用 demo 测试菜单）：
//   OK 短按   主页=一键起卦；卦象页=进入解卦；解卦页=滚到下一块
//   上/下     解卦页滚动；其它页切换信息块
//   OK 长按   任意页返回主页（可重新起卦）
#include "bagua_app.h"
#include "bagua_core.h"
#include "bagua_ui.h"
#include "key_guard.h"

#include "bsp_audio.h"
#include "bsp_battery.h"
#include "bsp_button.h"
#include "bsp_display.h"
#include "bsp_i2c.h"
#include "bsp_pins.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "lvgl.h"

#include <string.h>

static const char *TAG = "bagua";

#define INPUT_QUEUE_DEPTH 8
#define CAST_TICK_MS 280
#define KEY_GUARD_MAX_US (10LL * 1000 * 1000)

typedef struct {
    bsp_btn_t btn;
    bsp_btn_ev_t event;
} input_event_t;

static bagua_state_t s_state;
static QueueHandle_t s_input_queue;
static TaskHandle_t s_app_task;
static volatile bool s_input_ready;
static key_guard_t s_key_guard;
static esp_timer_handle_t s_cast_timer;
static int64_t s_idle_deadline_us;
static bool s_dimmed;

static uint32_t bagua_rng(void *ctx)
{
    (void)ctx;
    return esp_random();
}

static void cast_timer_stop(void)
{
    if (s_cast_timer) {
        esp_timer_stop(s_cast_timer);
    }
}

static void goto_home_locked(void)
{
    cast_timer_stop();
    memset(&s_state, 0, sizeof(s_state));
    s_state.page = BAGUA_PAGE_HOME;
    s_state.index = -1;
    s_state.changed = -1;
    bagua_ui_refresh(&s_state);
}

static void enter_result_locked(void)
{
    s_state.page = BAGUA_PAGE_RESULT;
    s_state.reveal_lines = 6;
    bagua_ui_refresh(&s_state);
}

static void on_cast_timer(void *arg)
{
    (void)arg;
    if (!s_input_queue) {
        return;
    }
    // 哨兵事件：PRESS 表示「定时器到点」，按当前 page 处理。
    input_event_t ev = { .btn = BSP_BTN_OK, .event = BSP_BTN_PRESS };
    (void)xQueueSend(s_input_queue, &ev, 0);
}

static void handle_timer_tick(void)
{
    if (!bsp_lvgl_lock(500)) {
        return;
    }
    if (s_state.page == BAGUA_PAGE_FOCUS) {
        if (s_state.focus_count > 1) {
            s_state.focus_count--;
            bagua_ui_refresh(&s_state);
        } else {
            // 倒计时结束 → 显爻
            s_state.focus_count = 0;
            s_state.page = BAGUA_PAGE_CASTING;
            s_state.reveal_lines = 0;
            bagua_ui_refresh(&s_state);
            if (s_cast_timer) {
                esp_timer_stop(s_cast_timer);
                esp_timer_start_periodic(s_cast_timer, (uint64_t)CAST_TICK_MS * 1000ull);
            }
        }
        bsp_lvgl_unlock();
        return;
    }
    if (s_state.page == BAGUA_PAGE_CASTING) {
        const bool done = bagua_ui_step_casting(&s_state);
        if (done) {
            cast_timer_stop();
            enter_result_locked();
        }
    }
    bsp_lvgl_unlock();
}

// 长按起卦：此刻取样六爻，再进入凝神倒计时。
static void start_ritual_locked(void)
{
    memset(&s_state.cast, 0, sizeof(s_state.cast));
    if (!bagua_cast_cast(&s_state.cast, bagua_rng, NULL)) {
        ESP_LOGE(TAG, "起卦失败");
        return;
    }
    s_state.index = bagua_index_from_lines(s_state.cast.lines);
    s_state.changed = bagua_changed_index(&s_state.cast);
    s_state.changing_count = bagua_changing_count(&s_state.cast);
    s_state.reveal_lines = 0;
    s_state.focus_count = 5;
    s_state.page = BAGUA_PAGE_FOCUS;
    bagua_ui_refresh(&s_state);
    if (s_cast_timer) {
        esp_timer_stop(s_cast_timer);
        esp_timer_start_periodic(s_cast_timer, 1000000ull); // 1s 一跳
    } else {
        // 无定时器则直接出结果
        enter_result_locked();
    }
}

static void handle_input(const input_event_t *in)
{
    // 定时器哨兵
    if (in->event == BSP_BTN_PRESS && in->btn == BSP_BTN_OK) {
        handle_timer_tick();
        return;
    }

    if (in->event == BSP_BTN_LONG && in->btn == BSP_BTN_OK) {
        if (!bsp_lvgl_lock(500)) {
            return;
        }
        s_dimmed = false;
        bsp_display_backlight(100);
        if (s_state.page == BAGUA_PAGE_HOME) {
            // 长按才起卦：此刻取样
            start_ritual_locked();
        } else {
            cast_timer_stop();
            goto_home_locked();
        }
        bsp_lvgl_unlock();
        return;
    }
    if (in->event != BSP_BTN_CLICK) {
        return;
    }

    if (!bsp_lvgl_lock(500)) {
        return;
    }
    s_dimmed = false;
    bsp_display_backlight(100);
    s_idle_deadline_us = esp_timer_get_time() + 30LL * 1000 * 1000;

    switch (s_state.page) {
    case BAGUA_PAGE_HOME:
        // 短按不起卦，保持仪式感
        break;
    case BAGUA_PAGE_FOCUS:
    case BAGUA_PAGE_CASTING:
        break;
    case BAGUA_PAGE_RESULT:
    case BAGUA_PAGE_READING:
        // 同一页：上=卦象，下=解卦
        if (in->btn == BSP_BTN_UP) {
            bagua_ui_scroll_reading(36);
        } else if (in->btn == BSP_BTN_DOWN) {
            bagua_ui_scroll_reading(-36);
        } else if (in->btn == BSP_BTN_OK) {
            goto_home_locked();
        }
        break;
    }
    bsp_lvgl_unlock();
}

static void input_task(void *arg)
{
    (void)arg;
    input_event_t ev;
    for (;;) {
        if (xQueueReceive(s_input_queue, &ev, portMAX_DELAY) == pdTRUE) {
            handle_input(&ev);
        }
    }
}

static void on_key(bsp_btn_t btn, bsp_btn_ev_t ev, void *user)
{
    (void)user;
    const bool was_armed = s_key_guard.armed;
    if (key_guard_consume(&s_key_guard, ev == BSP_BTN_RELEASE, esp_timer_get_time())) {
        if (was_armed && !s_key_guard.armed) {
            ESP_LOGI(TAG, "唤醒键已松手，按键恢复响应");
        }
        return;
    }
    if (!s_input_ready || !s_input_queue) {
        return;
    }
    const input_event_t input = { .btn = btn, .event = ev };
    (void)xQueueSend(s_input_queue, &input, 0);
}

static void idle_timer_poll(void *arg)
{
    (void)arg;
    if (s_dimmed || s_idle_deadline_us == 0) {
        return;
    }
    if (esp_timer_get_time() >= s_idle_deadline_us) {
        s_dimmed = true;
        bsp_display_backlight(18);
    }
}

esp_err_t bagua_app_start(void)
{
    ESP_LOGI(TAG, "八卦罗盘启动");
    const uint32_t wake_causes = esp_sleep_get_wakeup_causes();
    if (wake_causes & (1u << ESP_SLEEP_WAKEUP_GPIO)) {
        key_guard_arm(&s_key_guard, esp_timer_get_time(), KEY_GUARD_MAX_US);
    }

    bsp_i2c_init();
    if (bsp_display_init() != ESP_OK || !bsp_lvgl_init()) {
        ESP_LOGE(TAG, "显示/LVGL 初始化失败");
        return ESP_FAIL;
    }
    bsp_display_backlight(20);
    (void)bsp_audio_init();
    (void)bsp_battery_init();

    s_input_queue = xQueueCreate(INPUT_QUEUE_DEPTH, sizeof(input_event_t));
    if (!s_input_queue) {
        return ESP_ERR_NO_MEM;
    }
    if (xTaskCreate(input_task, "bagua_in", 4096, NULL, 5, &s_app_task) != pdPASS) {
        return ESP_ERR_NO_MEM;
    }

    // 按键先于 UI：失败也不要挡住界面。
    esp_err_t berr = bsp_button_init(on_key, NULL);
    ESP_LOGW(TAG, "bsp_button_init=%s", esp_err_to_name(berr));
    if (s_key_guard.armed) {
        const bool released = bsp_button_read_mv() >= BSP_BTN_MV_RELEASED_MIN;
        key_guard_sync_released(&s_key_guard, released);
        if (s_key_guard.armed) {
            // 兜底：初始化完成后不再无限吞键，最多再等 2s。
            s_key_guard.deadline_us = esp_timer_get_time() + 2000000;
        }
    }

    const esp_timer_create_args_t targs = {
        .callback = on_cast_timer,
        .name = "bagua_cast",
    };
    if (esp_timer_create(&targs, &s_cast_timer) != ESP_OK) {
        s_cast_timer = NULL;
    }
    esp_timer_handle_t idle_timer = NULL;
    const esp_timer_create_args_t iargs = {
        .callback = idle_timer_poll,
        .name = "bagua_idle",
    };
    if (esp_timer_create(&iargs, &idle_timer) == ESP_OK) {
        esp_timer_start_periodic(idle_timer, 1000000ull);
    }

    memset(&s_state, 0, sizeof(s_state));
    s_state.page = BAGUA_PAGE_HOME;
    s_state.index = -1;
    s_state.changed = -1;
    s_idle_deadline_us = esp_timer_get_time() + 30LL * 1000 * 1000;

    if (!bsp_lvgl_lock(1000)) {
        ESP_LOGE(TAG, "LVGL lock timeout");
        return ESP_ERR_TIMEOUT;
    }
    bagua_ui_create(&s_state);
    bagua_ui_set_battery(bsp_battery_soc());
    bsp_lvgl_unlock();
    bsp_display_backlight(100);
    s_input_ready = true;
    ESP_LOGI(TAG, "八卦罗盘就绪 input_ready=1");
    return ESP_OK;
}
