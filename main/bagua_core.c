// main/bagua_core.c —— 六爻起卦/解卦纯逻辑实现。
#include "bagua_core.h"

#include <stdio.h>
#include <string.h>

int bagua_hexagram_index(uint8_t hex_lines)
{
    const uint8_t u = bagua_upper_tri(hex_lines);
    const uint8_t l = bagua_lower_tri(hex_lines);
    const int kw = BAGUA_KING_WEN_BY_TRI[u][l];
    if (kw < 1 || kw > 64) {
        return -1;
    }
    return kw - 1;
}

int bagua_index_from_lines(const uint8_t lines[6])
{
    return bagua_hexagram_index(bagua_pack_lines(lines));
}

int bagua_changed_index(const bagua_cast_t *cast)
{
    if (!cast) {
        return -1;
    }
    uint8_t lines[6];
    for (int i = 0; i < 6; i++) {
        const bool chg = (cast->changing & (uint8_t)(1u << i)) != 0;
        lines[i] = chg ? (uint8_t)(!cast->lines[i] & 1u) : cast->lines[i];
    }
    return bagua_index_from_lines(lines);
}

int bagua_changing_count(const bagua_cast_t *cast)
{
    if (!cast) {
        return 0;
    }
    int n = 0;
    for (int i = 0; i < 6; i++) {
        if (cast->changing & (uint8_t)(1u << i)) {
            n++;
        }
    }
    return n;
}

void bagua_line_name(int idx, bool is_yang, char *out, int out_len)
{
    static const char *const POS[6][2] = {
        {"初六", "初九"}, {"六二", "九二"}, {"六三", "九三"},
        {"六四", "九四"}, {"六五", "九五"}, {"上六", "上九"},
    };
    if (!out || out_len <= 0) {
        return;
    }
    if (idx < 0 || idx > 5) {
        out[0] = '\0';
        return;
    }
    const char *s = POS[idx][is_yang ? 1 : 0];
    // 短标签 2 个汉字 = 6 字节 + NUL。
    snprintf(out, (size_t)out_len, "%s", s);
}

void bagua_value_to_line(uint8_t value, uint8_t *line, uint8_t *changing)
{
    // 6=老阴(动) 7=少阳 8=少阴 9=老阳(动)
    uint8_t y = 0;
    uint8_t chg = 0;
    switch (value) {
    case 6:
        y = 0;
        chg = 1;
        break;
    case 7:
        y = 1;
        chg = 0;
        break;
    case 8:
        y = 0;
        chg = 0;
        break;
    case 9:
        y = 1;
        chg = 1;
        break;
    default:
        y = 0;
        chg = 0;
        break;
    }
    if (line) {
        *line = y;
    }
    if (changing) {
        *changing = chg;
    }
}

bool bagua_cast_from_values(bagua_cast_t *out, const uint8_t values[6])
{
    if (!out || !values) {
        return false;
    }
    memset(out, 0, sizeof(*out));
    for (int i = 0; i < 6; i++) {
        if (values[i] < 6 || values[i] > 9) {
            return false;
        }
        out->values[i] = values[i];
        uint8_t y = 0;
        uint8_t chg = 0;
        bagua_value_to_line(values[i], &y, &chg);
        out->lines[i] = y;
        if (chg) {
            out->changing = (uint8_t)(out->changing | (1u << i));
        }
    }
    return true;
}

bool bagua_cast_cast(bagua_cast_t *out, uint32_t (*rng)(void *), void *ctx)
{
    if (!out || !rng) {
        return false;
    }
    uint8_t values[6];
    for (int i = 0; i < 6; i++) {
        // 三枚铜钱：每枚 bit0 随机 0=背(2)/1=字(3)，总和 6..9。
        const uint32_t r = rng(ctx);
        int sum = 0;
        for (int c = 0; c < 3; c++) {
            sum += ((r >> (c * 2)) & 1u) ? 3 : 2;
        }
        values[i] = (uint8_t)sum;
    }
    return bagua_cast_from_values(out, values);
}

void bagua_fill_line_labels(const bagua_cast_t *cast, char labels[6][8])
{
    if (!cast || !labels) {
        return;
    }
    for (int i = 0; i < 6; i++) {
        bagua_line_name(i, cast->lines[i] != 0, labels[i], 8);
    }
}
