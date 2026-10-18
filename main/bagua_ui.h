// main/bagua_ui.h —— 八卦罗盘界面（自研 UI，不复用 demo 测试壳）。
#pragma once

#include "bagua_core.h"
#include "bsp_button.h"
#include <stdbool.h>

// 页面阶段。
typedef enum {
    BAGUA_PAGE_HOME = 0,     // 罗盘主页
    BAGUA_PAGE_FOCUS,        // 凝神倒计时（取样已完成，尚未显爻）
    BAGUA_PAGE_CASTING,      // 起卦动画中
    BAGUA_PAGE_RESULT,       // 本卦 / 动爻 / 变卦
    BAGUA_PAGE_READING,      // 解卦正文（可滚动）
} bagua_page_t;

// 应用状态（UI 只读展示，切换由 app 调度）。
typedef struct {
    bagua_page_t page;
    bagua_cast_t cast;
    int index;          // 本卦下标
    int changed;        // 变卦下标
    int changing_count;
    int reveal_lines;   // 起卦动画：已显现的爻数 0..6
    int focus_count;    // 凝神倒计时 3..0，0 表示到时
    int scroll;         // 解卦滚动行
} bagua_state_t;

// 创建根屏幕与主页（须在 LVGL 任务 / bsp_lvgl_lock 内）。
void bagua_ui_create(const bagua_state_t *state);

// 销毁屏幕前先停掉定时器（app 在删除对象前调用）。
void bagua_ui_teardown(void);

// 按当前状态刷新可见页；进入结果/解卦时会按 cast 重建文案。
void bagua_ui_refresh(const bagua_state_t *state);

// 起卦动画：把 reveal_lines 从 0 推进到 6；返回 true 表示动画完成。
bool bagua_ui_step_casting(bagua_state_t *state);

// 电池指示（右上角，-1 隐藏数值）。
void bagua_ui_set_battery(int soc);

// 解卦页滚动（LVGL 锁内调用）。
void bagua_ui_scroll_reading(int dy);
