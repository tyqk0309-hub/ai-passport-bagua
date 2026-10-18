// main/bagua_font.c —— 字体描述符与 Latin/符号 fallback。
#include "bagua_font.h"

static lv_font_t s_body;
static lv_font_t s_title;
static bool s_ready;

void bagua_fonts_init(void)
{
    if (s_ready) {
        return;
    }
    s_body = bg_font_16;
    s_body.fallback = &lv_font_montserrat_14;
    s_title = bg_font_22;
    s_title.fallback = &lv_font_montserrat_20;
    s_ready = true;
}

const lv_font_t *bagua_font_body(void)
{
    return &s_body;
}

const lv_font_t *bagua_font_title(void)
{
    return &s_title;
}

bool bagua_font_has_glyph(const lv_font_t *font, uint32_t codepoint)
{
    if (!font) {
        return false;
    }
    lv_font_glyph_dsc_t glyph = {0};
    return lv_font_get_glyph_dsc(font, &glyph, codepoint, 0) && !glyph.is_placeholder;
}
