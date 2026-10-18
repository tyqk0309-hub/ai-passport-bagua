# 八卦罗盘（Bagua Compass）

<p align="right"><strong>简体中文</strong> · <a href="bagua-compass.md">English</a></p>

基于 [FoloToy AI Passport](https://gitee.com/FoloToy/ai-passport) 可穿戴硬件的**离线六爻卜卦**固件应用。

一枚 240×320 彩屏、三键、ESP32-C3 的小设备：长按起卦 → 凝神倒计时 → 成爻 → 卦象 / 解卦。

> 本应用为衍生固件：复用官方 BSP，独立设计界面，不依赖手机与网络。

## 功能

| 功能 | 说明 |
| --- | --- |
| 太极八卦主页 | 静态太极 + 先天八卦环（RGB565 底图） |
| 长按起卦 | 短按不起卦；长按 OK 瞬间取样六爻 |
| 凝神倒计时 | 5 → 4 → 3 → 2 → 1 后自下而上显爻 |
| 三枚铜钱法 | 6/7/8/9 对应老阴/少阳/少阴/老阳 |
| 六十四卦 | 文王卦序全表，卦辞 + 六爻爻辞 |
| 动爻 / 变卦 | 动爻标红 ※，附变卦名与变卦卦辞 |
| 解卦阅读 | 卦象与解卦同页，上键看卦象、下键看解卦 |

## 操作

| 按键 | 主页 | 卦象 / 解卦 |
| --- | --- | --- |
| **OK 长按** | 起卦 | 回主页 |
| **OK 短按** | — | 回主页 |
| **上键** | — | 向上滚（看卦象） |
| **下键** | — | 向下滚（看解卦） |

## 项目源码

### 应用逻辑（`main/`）

| 文件 | 职责 |
| --- | --- |
| `main.c` | 入口，调用 `bagua_app_start()` |
| `bagua_app.c/h` | 生命周期、按键、凝神/显爻定时器 |
| `bagua_core.c/h` | 纯逻辑：三钱起卦、查表、变卦、爻名 |
| `bagua_data.c` | 六十四卦数据（由脚本生成，勿手改） |
| `bagua_ui.c/h` | LVGL 界面：主页 / 凝神 / 显爻 / 卦象解卦 |
| `bagua_font.c/h` | 中文 16/22px 字体与 fallback |
| `bagua_img.c/h` | 太极八卦 RGB565 底图 |
| `bg_font_16.c` / `bg_font_22.c` | Noto Sans SC 子集字模 |

### 工具与测试

| 文件 | 职责 |
| --- | --- |
| `tools/bagua_texts.py` | 卦辞 / 爻辞文案源 |
| `tools/gen_bagua_data.py` | 生成 `bagua_data.c` |
| `tools/gen_bagua_img.py` | 生成太极八卦底图 |
| `tools/gen_bagua_symbols.py` | 提取中文用字清单 |
| `tests/test_bagua_core.c` | 起卦 / 查表 / 变卦主机测试 |

### 资源（`assets/fonts/`）

| 文件 | 说明 |
| --- | --- |
| `NotoSansSC-Regular.otf` | SIL OFL 1.1，允许再分发 |
| `bg_symbols.txt` | 用字清单 |

## 构建与刷写

环境：ESP-IDF **5.5.3**，目标 **ESP32-C3**（8MB Flash，无 PSRAM）。

```bash
# 主机逻辑测试
./tools/run_host_tests.sh

# 或完整门禁（静态 + 固件）
./tools/validate.sh

# 生成合并镜像（0x0 可刷写）
# Windows 亦可用 tools/build_firmware.ps1
./tools/validate.sh --firmware
```

产物：

```text
build/FoloToy-AI-Passport-full.bin
```

刷写（合并镜像写 **0x0**，会覆盖整片固件区，NVS 可能被重置）：

```bash
esptool --chip esp32c3 -p PORT -b 460800 write_flash \
  --flash_mode dio --flash_size 8MB --flash_freq 80m \
  0x0 build/FoloToy-AI-Passport-full.bin
```

## 硬件

| 项目 | 规格 |
| --- | --- |
| SoC | ESP32-C3，8MB Flash，无 PSRAM |
| 屏幕 | ST7789 240×320，SPI |
| 按键 | UP / DOWN / OK，ADC 分压 |
| 其它 | ES8311 音频、CW2017 电量（本应用可选） |

引脚以 `components/bsp/include/bsp_pins.h` 为准。

## 说明

- 起卦使用芯片硬件随机数（`esp_random`），保证公平，不含「感应」。
- 界面文案为现代白话短解，便于小屏阅读，非《周易》全文。
- 中文渲染依赖 Noto Sans SC 子集；新增文案需用 `tools/gen_bagua_symbols.py` 重新生成字模。

## 许可与致谢

- 硬件抽象与基线来自 [FoloToy AI Passport](https://gitee.com/FoloToy/ai-passport)（MIT）。
- 字体：Noto Sans CJK SC（SIL OFL 1.1）。
- 衍生应用代码可按仓库 MIT 条款使用；请保留上游版权声明。

---

自用可穿戴六爻罗盘 · 离线 · 开源可改
