// main/bagua_app.h —— 八卦罗盘应用入口。
#pragma once

#include "esp_err.h"

// 初始化 BSP + UI 并进入罗盘主页。失败返回错误码。
esp_err_t bagua_app_start(void);
