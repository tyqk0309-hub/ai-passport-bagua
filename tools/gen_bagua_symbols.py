#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Collect bagua UI Chinese codepoints and emit assets/fonts/bg_symbols.txt."""
from __future__ import annotations
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
SOURCES = [
    ROOT / "main" / "bagua_ui.c",
    ROOT / "main" / "bagua_core.c",
    ROOT / "main" / "bagua_data.c",
    ROOT / "main" / "bagua_app.c",
    ROOT / "main" / "bagua_font.c",
]
# Fixed UI literals that must be covered even if split across strings.
EXTRA = "八卦罗盘一键六爻卜卦解卦起卦中掷钱成爻自下而上卦象变卦上下键滚动长按返回本卦建议顺时而为中正守心无动爻以卦辞为主安静十六初九六二三四五上乾坤震巽坎离艮兑天泽火山风水山"

def main() -> int:
    chars: set[str] = set()
    for path in SOURCES:
        text = path.read_text(encoding="utf-8")
        for ch in text:
            o = ord(ch)
            if 0x4E00 <= o <= 0x9FFF or 0x3000 <= o <= 0x303F or 0xFF00 <= o <= 0xFFEF:
                chars.add(ch)
            elif 0x20 <= o <= 0x7E:
                chars.add(ch)
    for ch in EXTRA:
        chars.add(ch)
    # Stable order: ASCII then CJK by codepoint.
    ordered = sorted(chars, key=lambda c: (ord(c) > 0x7F, ord(c)))
    raw = "".join(ordered)
    out_txt = ROOT / "assets" / "fonts" / "bg_symbols.txt"
    out_raw = ROOT / "assets" / "fonts" / "bg_symbols_raw.txt"
    # bg_symbols.txt: readable inventory with spaces between CJK for docs
    pretty = []
    for ch in ordered:
        pretty.append(ch)
    out_txt.write_text("".join(pretty), encoding="utf-8")
    out_raw.write_text(raw, encoding="utf-8")
    print(f"unique={len(ordered)} -> {out_txt}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
