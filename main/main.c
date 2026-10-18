// main/main.c —— 八卦罗盘入口。
#include "bagua_app.h"
#include "esp_log.h"

static const char *TAG = "main";

void app_main(void)
{
    esp_err_t err = bagua_app_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "bagua_app_start failed: %s", esp_err_to_name(err));
    }
}
