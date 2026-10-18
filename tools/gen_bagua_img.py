#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Draw taiji + 先天八卦 ring as RGB565 LVGL image (240x320)."""
from __future__ import annotations
import math
import pathlib

W, H = 240, 320
CX, CY = 120, 150
R = 56
RING = 94

BG = (11, 18, 32)
YIN = (26, 36, 56)
YANG = (232, 220, 200)
GOLD = (212, 160, 23)

# bit0=底爻, 1=阳
# 先天八卦：乾南(上)，顺时针 乾兑离震坤艮坎巽
ORDER = [7, 3, 5, 1, 0, 4, 2, 6]


def rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


class Img:
    def __init__(self):
        self.pix = [[BG for _ in range(W)] for _ in range(H)]

    def set(self, x, y, c):
        if 0 <= x < W and 0 <= y < H:
            self.pix[y][x] = c

    def fill_circle(self, cx, cy, r, c):
        rr = r * r
        for y in range(int(cy - r) - 1, int(cy + r) + 2):
            for x in range(int(cx - r) - 1, int(cx + r) + 2):
                dx, dy = x + 0.5 - cx, y + 0.5 - cy
                if dx * dx + dy * dy <= rr:
                    self.set(x, y, c)

    def ring(self, cx, cy, r, half_w, c):
        for y in range(int(cy - r - half_w) - 1, int(cy + r + half_w) + 2):
            for x in range(int(cx - r - half_w) - 1, int(cx + r + half_w) + 2):
                dx, dy = x + 0.5 - cx, y + 0.5 - cy
                d = math.hypot(dx, dy)
                if abs(d - r) <= half_w:
                    self.set(x, y, c)


def draw_taiji(img: Img, cx: float, cy: float, r: float):
    # 标准画法：左右分割 + 两个骑在中线上的圆形成 S。
    # 1) 整圆阳（浅）  2) 左半阴  3) 中线上阴圆向下伸  4) 中线上阳圆向上伸
    for y in range(int(cy - r) - 2, int(cy + r) + 3):
        for x in range(int(cx - r) - 2, int(cx + r) + 3):
            dx, dy = x + 0.5 - cx, y + 0.5 - cy
            if dx * dx + dy * dy > r * r:
                continue
            half = r / 2.0
            # 上圆：圆心 (cx, cy-half)，半径 half —— 骑在竖直中线上
            if dx * dx + (dy + half) ** 2 <= half * half:
                img.set(x, y, YIN)          # 阴头伸入右侧
            elif dx * dx + (dy - half) ** 2 <= half * half:
                img.set(x, y, YANG)         # 阳头伸入左侧
            elif dx < 0:
                img.set(x, y, YIN)          # 左半阴
            else:
                img.set(x, y, YANG)         # 右半阳
    # 鱼眼：阴中有阳，阳中有阴
    eye = max(4, int(r / 5.5))
    img.fill_circle(cx, cy - r / 2, eye, YANG)   # 上圆中心 = 阳眼
    img.fill_circle(cx, cy + r / 2, eye, YIN)    # 下圆中心 = 阴眼
    img.ring(cx, cy, r + 1, 2.0, GOLD)


def draw_trigram(img: Img, cx: float, cy: float, bits: int):
    w, h, gap = 28, 5, 9
    for i in range(3):
        yang = (bits >> i) & 1
        y = int(cy + (1 - i) * 10 - h / 2)
        x0 = int(cx - w / 2)
        if yang:
            for yy in range(y, y + h):
                for xx in range(x0, x0 + w):
                    img.set(xx, yy, GOLD)
        else:
            half = (w - gap) // 2
            for yy in range(y, y + h):
                for xx in range(x0, x0 + half):
                    img.set(xx, yy, GOLD)
                for xx in range(x0 + w - half, x0 + w):
                    img.set(xx, yy, GOLD)


def main():
    img = Img()
    draw_taiji(img, CX, CY, R)
    for i, bits in enumerate(ORDER):
        ang = -math.pi / 2 + i * (math.pi / 4)
        draw_trigram(img, CX + RING * math.cos(ang), CY + RING * math.sin(ang), bits)

    rows = []
    for y in range(H):
        cells = []
        for x in range(W):
            r, g, b = img.pix[y][x]
            v = rgb565(r, g, b)
            cells.append(f"0x{v & 0xFF:02x},0x{v >> 8:02x}")
        rows.append("    " + ",".join(cells) + ",")

    out = pathlib.Path("main/bagua_img.c")
    out.write_text(
        "// tools/gen_bagua_img.py 生成：太极 + 先天八卦 240x320 RGB565\n"
        '#include "bagua_img.h"\n\n'
        f"const uint8_t bagua_img_map[{W * H * 2}] = {{\n"
        + "\n".join(rows)
        + "\n};\n\n"
        "const lv_image_dsc_t bagua_img = {\n"
        "    .header = {\n"
        "        .magic = LV_IMAGE_HEADER_MAGIC,\n"
        "        .cf = LV_COLOR_FORMAT_RGB565,\n"
        "        .flags = 0,\n"
        f"        .w = {W},\n"
        f"        .h = {H},\n"
        f"        .stride = {W * 2},\n"
        "    },\n"
        f"    .data_size = {W * H * 2},\n"
        "    .data = bagua_img_map,\n"
        "};\n",
        encoding="utf-8",
    )
    print("wrote", out, out.stat().st_size)


if __name__ == "__main__":
    main()
