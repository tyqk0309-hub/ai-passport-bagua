# Bagua Compass (八卦罗盘)

<p align="right"><a href="bagua-compass.zh_CN.md">简体中文</a> · <strong>English</strong></p>

An **offline six-line (Liuyao) divination** firmware app for the
[FoloToy AI Passport](https://gitee.com/FoloToy/ai-passport) wearable.

A 240×320 color display, three buttons, ESP32-C3: long-press to cast →
focus countdown → lines appear → hexagram and interpretation.

> Derivative firmware: reuses the official BSP, fully custom UI, no phone or network.

## Features

| Feature | Description |
| --- | --- |
| Taiji + Bagua home | Static taiji and Earlier-Heaven trigram ring (RGB565) |
| Long-press cast | Short press does nothing; long OK samples all six lines |
| Focus countdown | 5→4→3→2→1, then lines reveal bottom to top |
| Three-coin method | 6/7/8/9 = old yin / young yang / young yin / old yang |
| 64 hexagrams | King Wen order, judgment + six line texts |
| Changing lines | Red ※ marks, relating hexagram name and judgment |
| Reading | Hexagram and interpretation in one scrollable view |

## Controls

| Button | Home | Hexagram / Reading |
| --- | --- | --- |
| **OK long** | Cast | Home |
| **OK short** | — | Home |
| **Up** | — | Scroll up (hexagram) |
| **Down** | — | Scroll down (interpretation) |

## Source layout

See the Chinese page [`bagua-compass.zh_CN.md`](bagua-compass.zh_CN.md) for the full file table.

Key paths:

- `main/bagua_*.c` — application, pure logic, UI, fonts, image
- `tools/gen_bagua_*.py` — data / image / glyph generators
- `tools/bagua_texts.py` — hexagram text source
- `tests/test_bagua_core.c` — host tests

## Build & flash

ESP-IDF **5.5.3**, target **ESP32-C3** (8MB Flash, no PSRAM).

```bash
./tools/run_host_tests.sh
./tools/validate.sh --firmware
```

Flash the merged image at **0x0**:

```text
build/FoloToy-AI-Passport-full.bin
```

## License

Based on FoloToy AI Passport (MIT). Font: Noto Sans CJK SC (SIL OFL 1.1).
