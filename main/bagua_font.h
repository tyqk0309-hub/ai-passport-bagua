// main/bagua_font.h —— 八卦罗盘中文子集字体（16/22px）。
#pragma once

#include "lvgl.h"

LV_FONT_DECLARE(bg_font_16);
LV_FONT_DECLARE(bg_font_22);

// 在 LVGL 初始化后、创建任何 label 前调用一次（LVGL 任务或持 bsp_lvgl_lock）。
void bagua_fonts_init(void);

// 当前应用正文/标题字体。
const lv_font_t *bagua_font_body(void);
const lv_font_t *bagua_font_title(void);

// 覆盖检查：返回 true 表示字体（含 fallback）含该码点且非占位符。
bool bagua_font_has_glyph(const lv_font_t *font, uint32_t codepoint);
