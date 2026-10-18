// main/bagua_ui.c —— 八卦罗盘界面：单 screen，分页惰性创建。
// 卦象与解卦在同一页上下滚动，避免按键切换把卦象切没。
#include "bagua_ui.h"
#include "bagua_font.h"
#include "bagua_img.h"
#include "bsp_display.h"
#include "esp_log.h"
#include "lvgl.h"

#include <stdio.h>
#include <string.h>

static const char *TAG = "bagua_ui";

#define UI_BG        0x0B1220
#define UI_PANEL     0x142033
#define UI_GOLD      0xD4A017
#define UI_GOLD_DIM  0x8A6B18
#define UI_PARCH     0xE8DCC8
#define UI_MUTED     0x8B9DC3
#define UI_RED       0xE85D4C
#define UI_YIN       0x1A2438
#define UI_YANG      0xE8DCC8

static lv_obj_t *s_scr;
static lv_obj_t *s_page[5];
static lv_obj_t *s_read_body;
static int s_batt_soc = -1;

static lv_obj_t *make_label(lv_obj_t *parent, const lv_font_t *font, uint32_t color, int w)
{
    lv_obj_t *lab = lv_label_create(parent);
    lv_obj_set_style_text_font(lab, font, 0);
    lv_obj_set_style_text_color(lab, lv_color_hex(color), 0);
    lv_obj_set_width(lab, w);
    lv_label_set_long_mode(lab, LV_LABEL_LONG_WRAP);
    return lab;
}

static lv_obj_t *ensure_page(int idx)
{
    if (s_page[idx]) {
        return s_page[idx];
    }
    lv_obj_t *p = lv_obj_create(s_scr);
    lv_obj_remove_style_all(p);
    lv_obj_set_size(p, 240, 320);
    lv_obj_set_style_bg_color(p, lv_color_hex(UI_BG), 0);
    lv_obj_set_style_bg_opa(p, LV_OPA_COVER, 0);
    lv_obj_add_flag(p, LV_OBJ_FLAG_HIDDEN);
    s_page[idx] = p;
    return p;
}

static void draw_yao(lv_obj_t *parent, int x, int y, int w, bool yang, bool moving)
{
    const uint32_t col = moving ? UI_RED : (yang ? UI_GOLD : UI_YIN);
    const int gap = 8;
    if (yang) {
        lv_obj_t *seg = lv_obj_create(parent);
        lv_obj_remove_style_all(seg);
        lv_obj_set_pos(seg, x, y);
        lv_obj_set_size(seg, w, 6);
        lv_obj_set_style_bg_color(seg, lv_color_hex(col), 0);
        lv_obj_set_style_bg_opa(seg, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(seg, 1, 0);
    } else {
        const int half = (w - gap) / 2;
        lv_obj_t *l = lv_obj_create(parent);
        lv_obj_remove_style_all(l);
        lv_obj_set_pos(l, x, y);
        lv_obj_set_size(l, half, 6);
        lv_obj_set_style_bg_color(l, lv_color_hex(col), 0);
        lv_obj_set_style_bg_opa(l, LV_OPA_COVER, 0);
        lv_obj_t *r = lv_obj_create(parent);
        lv_obj_remove_style_all(r);
        lv_obj_set_pos(r, x + w - half, y);
        lv_obj_set_size(r, half, 6);
        lv_obj_set_style_bg_color(r, lv_color_hex(col), 0);
        lv_obj_set_style_bg_opa(r, LV_OPA_COVER, 0);
    }
}

static void show_only(int idx)
{
    for (int i = 0; i < 5; i++) {
        if (!s_page[i]) {
            continue;
        }
        if (i == idx) {
            lv_obj_clear_flag(s_page[i], LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(s_page[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void build_home(void)
{
    lv_obj_t *p = ensure_page(0);
    lv_obj_t *img = lv_image_create(p);
    lv_image_set_src(img, &bagua_img);
    lv_obj_center(img);

    lv_obj_t *hint = make_label(p, bagua_font_body(), UI_MUTED, 160);
    lv_obj_set_style_text_align(hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -18);
    lv_label_set_text(hint, "长按起卦");
}

static void build_focus(const bagua_state_t *state)
{
    lv_obj_t *p = ensure_page(1);
    lv_obj_t *img = lv_image_create(p);
    lv_image_set_src(img, &bagua_img);
    lv_obj_center(img);

    lv_obj_t *cap = make_label(p, bagua_font_body(), UI_MUTED, 160);
    lv_obj_set_style_text_align(cap, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(cap, LV_ALIGN_TOP_MID, 0, 28);
    lv_label_set_text(cap, "凝神");

    lv_obj_t *num = make_label(p, bagua_font_title(), UI_GOLD, 120);
    lv_obj_set_style_text_align(num, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(num, LV_ALIGN_BOTTOM_MID, 0, -24);
    const int n = (state && state->focus_count > 0) ? state->focus_count : 5;
    lv_label_set_text_fmt(num, "%d", n);
}

static void build_casting(const bagua_state_t *state)
{
    lv_obj_t *p = ensure_page(2);
    (void)state;
    for (int i = 0; i < 6; i++) {
        const int y = 250 - i * 28;
        draw_yao(p, 70, y, 100, false, false);
        lv_obj_t *m = lv_label_create(p);
        lv_obj_set_style_text_font(m, bagua_font_body(), 0);
        lv_obj_set_style_text_color(m, lv_color_hex(UI_MUTED), 0);
        lv_obj_set_pos(m, 36, y - 2);
        lv_label_set_text(m, "");
    }
}

// 卦象 + 解卦：同一可滚容器，上下键滚动。
static void build_result(const bagua_state_t *state)
{
    lv_obj_t *p = ensure_page(3);
    const int idx = (state->index >= 0) ? state->index : 0;
    const bagua_hexagram_t *hex = &BAGUA_HEXAGRAMS[idx];

    lv_obj_t *body = lv_obj_create(p);
    s_read_body = body;
    lv_obj_set_size(body, 228, 308);
    lv_obj_align(body, LV_ALIGN_TOP_MID, 0, 6);
    lv_obj_set_style_bg_color(body, lv_color_hex(UI_BG), 0);
    lv_obj_set_style_bg_opa(body, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(body, 4, 0);
    lv_obj_set_style_pad_row(body, 3, 0);
    lv_obj_set_scroll_dir(body, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(body, LV_SCROLLBAR_MODE_AUTO);
    // 纵向排布，避免子控件叠在一点
    lv_obj_set_flex_flow(body, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(body, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

    lv_obj_t *name = make_label(body, bagua_font_title(), UI_PARCH, 216);
    lv_obj_set_style_text_align(name, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(name, hex->name);

    const bagua_trigram_t *up = &BAGUA_TRIGRAMS[hex->upper];
    const bagua_trigram_t *dn = &BAGUA_TRIGRAMS[hex->lower];
    lv_obj_t *tri = make_label(body, bagua_font_body(), UI_MUTED, 216);
    lv_obj_set_style_text_align(tri, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text_fmt(tri, "上%s(%s) 下%s(%s)",
                          up->name, up->nature, dn->name, dn->nature);

    for (int i = 5; i >= 0; i--) {
        const bool yang = state->cast.lines[i] != 0;
        const bool moving = (state->cast.changing & (1u << i)) != 0;
        lv_obj_t *row = lv_obj_create(body);
        lv_obj_set_size(row, 216, 22);
        lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
        lv_obj_set_style_pad_all(row, 0, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        char lname[8];
        bagua_line_name(i, yang, lname, sizeof(lname));
        lv_obj_t *m = lv_label_create(row);
        lv_obj_set_style_text_font(m, bagua_font_body(), 0);
        lv_obj_set_style_text_color(m, lv_color_hex(moving ? UI_RED : UI_MUTED), 0);
        lv_obj_set_pos(m, 4, 2);
        lv_label_set_text(m, lname);
        draw_yao(row, 56, 8, 120, yang, moving);
        if (moving) {
            lv_obj_t *st = lv_label_create(row);
            lv_obj_set_style_text_font(st, bagua_font_body(), 0);
            lv_obj_set_style_text_color(st, lv_color_hex(UI_RED), 0);
            lv_obj_set_pos(st, 186, 2);
            lv_label_set_text(st, "动");
        }
    }

    lv_obj_t *sep = make_label(body, bagua_font_title(), UI_GOLD, 216);
    lv_obj_set_style_text_align(sep, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(sep, "解卦");

    static char buf[2048];
    int n = snprintf(buf, sizeof(buf), "【本卦】%s\n%s\n\n", hex->name, hex->gua_ci);
    if (n < 0) {
        n = 0;
    }
    const bool has_chg = state->changing_count > 0;
    if (has_chg) {
        n += snprintf(buf + n, sizeof(buf) - (size_t)n, "【动爻】\n");
        for (int i = 0; i < 6 && n < (int)sizeof(buf) - 80; i++) {
            if (state->cast.changing & (1u << i)) {
                char lname[8];
                bagua_line_name(i, state->cast.lines[i] != 0, lname, sizeof(lname));
                n += snprintf(buf + n, sizeof(buf) - (size_t)n, "%s %s\n", lname, hex->yao[i]);
            }
        }
    } else {
        n += snprintf(buf + n, sizeof(buf) - (size_t)n, "【六爻】安静，以本卦卦辞为主。\n");
    }
    n += snprintf(buf + n, sizeof(buf) - (size_t)n, "\n【爻辞】\n");
    for (int i = 0; i < 6 && n < (int)sizeof(buf) - 80; i++) {
        char lname[8];
        bagua_line_name(i, state->cast.lines[i] != 0, lname, sizeof(lname));
        const bool moving = (state->cast.changing & (1u << i)) != 0;
        n += snprintf(buf + n, sizeof(buf) - (size_t)n, "%s%s %s\n",
                      moving ? "※" : "", lname, hex->yao[i]);
    }
    if (has_chg && state->changed >= 0) {
        const bagua_hexagram_t *ch = &BAGUA_HEXAGRAMS[state->changed];
        n += snprintf(buf + n, sizeof(buf) - (size_t)n, "\n【变卦】%s\n%s\n",
                      ch->name, ch->gua_ci);
        n += snprintf(buf + n, sizeof(buf) - (size_t)n,
                      "\n【断】本卦看现状，动爻示转折，变卦示趋向。\n");
    }
    if (n > (int)sizeof(buf) - 1) {
        n = (int)sizeof(buf) - 1;
    }
    buf[n] = '\0';
    lv_obj_t *lab = make_label(body, bagua_font_body(), UI_PARCH, 210);
    lv_label_set_text(lab, buf);
}

static void build_reading(const bagua_state_t *state)
{
    // 解卦已并入卦象页（build_result），此页不再单独使用。
    (void)state;
}

void bagua_ui_create(const bagua_state_t *state)
{
    ESP_LOGI(TAG, "ui_create begin");
    bagua_fonts_init();
    s_scr = lv_obj_create(NULL);
    lv_obj_remove_style_all(s_scr);
    lv_obj_set_size(s_scr, 240, 320);
    lv_obj_set_style_bg_color(s_scr, lv_color_hex(UI_BG), 0);
    lv_obj_set_style_bg_opa(s_scr, LV_OPA_COVER, 0);
    build_home();
    lv_screen_load(s_scr);
    show_only(0);
    ESP_LOGI(TAG, "ui_create done");
    (void)state;
}

void bagua_ui_teardown(void)
{
    s_scr = NULL;
    s_read_body = NULL;
    for (int i = 0; i < 5; i++) {
        s_page[i] = NULL;
    }
}

void bagua_ui_set_battery(int soc)
{
    s_batt_soc = soc;
}

bool bagua_ui_step_casting(bagua_state_t *state)
{
    if (!state) {
        return true;
    }
    if (state->reveal_lines < 6) {
        state->reveal_lines++;
    }
    return state->reveal_lines >= 6;
}

void bagua_ui_scroll_reading(int dy)
{
    if (s_read_body) {
        lv_obj_scroll_by(s_read_body, 0, dy, LV_ANIM_ON);
    }
}

void bagua_ui_refresh(const bagua_state_t *state)
{
    if (!state) {
        return;
    }
    switch (state->page) {
    case BAGUA_PAGE_HOME:
        if (!s_page[0]) {
            build_home();
        }
        show_only(0);
        break;
    case BAGUA_PAGE_FOCUS:
        if (s_page[1]) {
            lv_obj_clean(s_page[1]);
        }
        build_focus(state);
        show_only(1);
        break;
    case BAGUA_PAGE_CASTING:
        if (!s_page[2]) {
            build_casting(state);
        }
        show_only(2);
        break;
    case BAGUA_PAGE_RESULT:
    case BAGUA_PAGE_READING:
        // 同一页：卦象在上、解卦在下，上下键滚动。
        if (s_page[3]) {
            lv_obj_clean(s_page[3]);
        }
        s_read_body = NULL;
        build_result(state);
        show_only(3);
        break;
    }
}
