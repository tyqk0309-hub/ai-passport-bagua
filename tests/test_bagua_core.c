// tests/test_bagua_core.c —— 六爻起卦/识别/变卦纯逻辑主机测试。
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "bagua_core.h"

static uint32_t seq_rng(void *ctx)
{
    static uint32_t n;
    uint32_t *seed = (uint32_t *)ctx;
    if (seed) {
        *seed = *seed * 1664525u + 1013904223u;
        return *seed;
    }
    n += 1;
    return n * 2654435761u;
}

static void test_trigram_pack(void)
{
    // 乾 111
    uint8_t qian[6] = {1, 1, 1, 1, 1, 1};
    int idx = bagua_index_from_lines(qian);
    assert(idx == 0);
    assert(BAGUA_HEXAGRAMS[idx].king_wen == 1);
    assert(strcmp(BAGUA_HEXAGRAMS[idx].name, "乾为天") == 0);

    // 坤 000
    uint8_t kun[6] = {0, 0, 0, 0, 0, 0};
    idx = bagua_index_from_lines(kun);
    assert(BAGUA_HEXAGRAMS[idx].king_wen == 2);

    // 天风姤 = 上乾下巽。巽=011? 下卦巽：下爻阴 中阳 上阳 → lines[0]=0,1,2=0,1,1
    uint8_t gou[6] = {0, 1, 1, 1, 1, 1};
    idx = bagua_index_from_lines(gou);
    assert(strcmp(BAGUA_HEXAGRAMS[idx].name, "天风姤") == 0);
    assert(BAGUA_HEXAGRAMS[idx].king_wen == 44);

    // 水火既济 = 上坎下离。离=101, 坎=010
    uint8_t ji[6] = {1, 0, 1, 0, 1, 0};
    idx = bagua_index_from_lines(ji);
    assert(strcmp(BAGUA_HEXAGRAMS[idx].name, "水火既济") == 0);
    assert(BAGUA_HEXAGRAMS[idx].king_wen == 63);
}

static void test_lookup_complete(void)
{
    // 64 个文王卦序应一一对应 8x8。
    int seen[65] = {0};
    int count = 0;
    for (int u = 0; u < 8; u++) {
        for (int l = 0; l < 8; l++) {
            int kw = BAGUA_KING_WEN_BY_TRI[u][l];
            assert(kw >= 1 && kw <= 64);
            assert(seen[kw] == 0);
            seen[kw] = 1;
            count++;
        }
    }
    assert(count == 64);
}

static void test_values(void)
{
    bagua_cast_t cast;
    const uint8_t vals[6] = {9, 8, 7, 6, 9, 8};
    assert(bagua_cast_from_values(&cast, vals));
    assert(cast.lines[0] == 1 && (cast.changing & 1));
    assert(cast.lines[1] == 0 && (cast.changing & 2) == 0);
    assert(cast.lines[2] == 1 && (cast.changing & 4) == 0);
    assert(cast.lines[3] == 0 && (cast.changing & 8));
    assert(bagua_changing_count(&cast) == 3);

    // 变卦：动爻翻转。
    int born = bagua_index_from_lines(cast.lines);
    int changed = bagua_changed_index(&cast);
    assert(born >= 0 && changed >= 0);
    // 初爻、四爻、五爻翻转后与本卦不同（除非碰巧同卦，这里数值保证不同）。
    uint8_t expect[6] = {0, 0, 1, 1, 0, 0};
    assert(bagua_index_from_lines(expect) == changed);
}

static void test_invalid(void)
{
    uint8_t bad[6] = {0, 0, 0, 0, 0, 0};
    bagua_cast_t cast;
    assert(!bagua_cast_cast(NULL, seq_rng, NULL));
    assert(!bagua_cast_cast(&cast, NULL, NULL));
    assert(!bagua_cast_from_values(NULL, bad));
    const uint8_t oob[6] = {6, 6, 6, 6, 6, 5};
    assert(!bagua_cast_from_values(&cast, oob));
}

static void test_line_names(void)
{
    char buf[8];
    bagua_line_name(0, true, buf, sizeof(buf));
    assert(strcmp(buf, "初九") == 0);
    bagua_line_name(0, false, buf, sizeof(buf));
    assert(strcmp(buf, "初六") == 0);
    bagua_line_name(5, true, buf, sizeof(buf));
    assert(strcmp(buf, "上九") == 0);
    bagua_line_name(3, false, buf, sizeof(buf));
    assert(strcmp(buf, "六四") == 0);
}

static void test_cast_rng(void)
{
    uint32_t seed = 42;
    bagua_cast_t cast;
    assert(bagua_cast_cast(&cast, seq_rng, &seed));
    for (int i = 0; i < 6; i++) {
        assert(cast.values[i] >= 6 && cast.values[i] <= 9);
        assert(cast.lines[i] == 0 || cast.lines[i] == 1);
    }
    char labels[6][8];
    bagua_fill_line_labels(&cast, labels);
    for (int i = 0; i < 6; i++) {
        assert(labels[i][0] != '\0');
    }
}

// 变卦与本卦在无动爻时应相同。
static void test_no_change(void)
{
    bagua_cast_t cast;
    const uint8_t vals[6] = {7, 8, 7, 8, 7, 8};
    assert(bagua_cast_from_values(&cast, vals));
    assert(bagua_changing_count(&cast) == 0);
    assert(bagua_changed_index(&cast) == bagua_index_from_lines(cast.lines));
}

int main(void)
{
    test_trigram_pack();
    test_lookup_complete();
    test_values();
    test_invalid();
    test_line_names();
    test_cast_rng();
    test_no_change();
    puts("test_bagua_core: PASS");
    return 0;
}
