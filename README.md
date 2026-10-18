# 八卦罗盘 · Bagua Compass

基于 **FoloToy AI Passport** 可穿戴设备的离线六爻卜卦玩法。

> 长按起卦 → 凝神倒计时 → 成爻 → 卦象 / 解卦  
> 无需配网，离线可用

## 玩法简介

- **太极八卦主页**：开机即见太极与先天八卦
- **长按起卦**：一键三枚铜钱法，当场取样六爻
- **凝神 5 秒**：倒计时结束后自下而上显爻
- **六十四卦**：卦辞 + 六爻爻辞（白话短解）
- **动爻 / 变卦**：动爻标红，附变卦与解卦
- **上下翻阅**：上键看卦象，下键看解卦

## 使用方法

1. 开机即可使用，无需配网
2. 主页**长按 OK** 起卦
3. 凝神 5 秒后六爻显现
4. **上键**看卦象，**下键**看解卦
5. **OK** 回主页，可再次起卦

## 构建

需 ESP-IDF 5.5.3（ESP32-C3）：

```bash
./tools/run_host_tests.sh
./tools/validate.sh --firmware
# 产物：build/FoloToy-AI-Passport-full.bin
```

合并镜像从 **0x0** 刷写。

## 源码

| 路径 | 说明 |
| --- | --- |
| `main/bagua_*` | 应用逻辑、界面、字体、底图 |
| `tools/gen_bagua_*` / `bagua_texts.py` | 数据与素材生成 |
| `tests/test_bagua_core.c` | 主机测试 |
| `docs/bagua-compass.zh_CN.md` | 详细说明 |

## 致谢与许可

- 硬件基线：[FoloToy AI Passport](https://gitee.com/FoloToy/ai-passport)（MIT）
- 字体：Noto Sans CJK SC（SIL OFL 1.1）
