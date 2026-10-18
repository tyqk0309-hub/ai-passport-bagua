// main/bagua_core.h —— 六爻起卦/解卦纯逻辑（无 ESP-IDF / LVGL 依赖，可主机测试）。
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 单爻：0 = 阴，1 = 阳。卦象 lines 的 bit0 为初爻（最下），bit5 为上爻。
typedef struct {
    uint8_t lines[6];   // 阴阳
    uint8_t changing;   // bit i = 第 i 爻为动爻
    uint8_t values[6];   // 6=老阴 7=少阳 8=少阴 9=老阳
} bagua_cast_t;

typedef struct {
    uint8_t lines;      // 三位：bit0 下 / bit1 中 / bit2 上，1=阳
    const char *name;   // 乾/兑/离/震/巽/坎/艮/坤
    const char *nature; // 天/泽/火/雷/风/水/山/地
} bagua_trigram_t;

typedef struct {
    int king_wen;       // 文王卦序 1..64
    const char *name;   // 如「乾为天」「天风姤」
    uint8_t upper;      // 上卦三位模式
    uint8_t lower;      // 下卦三位模式
    const char *gua_ci; // 卦辞（现代短解）
    const char *yao[6]; // 六爻爻辞（下→上）
} bagua_hexagram_t;

extern const bagua_trigram_t BAGUA_TRIGRAMS[8];
extern const bagua_hexagram_t BAGUA_HEXAGRAMS[64];
// [upper][lower] -> 文王卦序 1..64
extern const uint8_t BAGUA_KING_WEN_BY_TRI[8][8];

// 由六爻阴阳码取下卦/上卦三位模式。
static inline uint8_t bagua_lower_tri(uint8_t hex_lines) {
    return (uint8_t)(hex_lines & 0x07u);
}
static inline uint8_t bagua_upper_tri(uint8_t hex_lines) {
    return (uint8_t)((hex_lines >> 3) & 0x07u);
}

// 由六爻（bit0=初爻）打包成卦象码。
static inline uint8_t bagua_pack_lines(const uint8_t lines[6]) {
    uint8_t v = 0;
    for (int i = 0; i < 6; i++) {
        if (lines[i]) {
            v = (uint8_t)(v | (1u << i));
        }
    }
    return v;
}

// 查表：卦象码 → 六十四卦下标 0..63；非法返回 -1。
int bagua_hexagram_index(uint8_t hex_lines);

// 由阴阳数组（可含动爻）得到本卦下标；全 0 输入返回 -1。
int bagua_index_from_lines(const uint8_t lines[6]);

// 动爻变卦：changing 位为 1 的爻翻转，得到变卦下标。
// 无动爻时变卦下标等于本卦下标。
int bagua_changed_index(const bagua_cast_t *cast);

// 动爻数量。
int bagua_changing_count(const bagua_cast_t *cast);

// 爻位名：初九/初六/九二/六二/.../上九/上六。idx 0..5。
// out 至少 8 字节（UTF-8 中文 3 字节×2 + NUL）。
void bagua_line_name(int idx, bool is_yang, char *out, int out_len);

// 传统三枚铜钱：每枚 2（背/阴）或 3（字/阳）；单爻总和 6/7/8/9。
// rng 返回任意 32 位随机数；返回 false 表示 rng 为 NULL。
bool bagua_cast_cast(bagua_cast_t *out, uint32_t (*rng)(void *), void *ctx);

// 由单爻数值（6/7/8/9）填充 cast；越界返回 false。
bool bagua_cast_from_values(bagua_cast_t *out, const uint8_t values[6]);

// 爻值 6/7/8/9 → 阴阳/是否动爻。
void bagua_value_to_line(uint8_t value, uint8_t *line, uint8_t *changing);

// 生成六爻显示标签列表（解卦用）。
// labels[i] 至少 8 字节。
void bagua_fill_line_labels(const bagua_cast_t *cast, char labels[6][8]);

#ifdef __cplusplus
}
#endif
