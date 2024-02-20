#pragma once

#include "esp.h"

#define ESP_ERR_NVS_NO_FREE_PAGES ESP_FAIL
#define ESP_ERR_NVS_NEW_VERSION_FOUND ESP_FAIL

esp_err_t nvs_flash_init() { return ESP_FAIL; }
esp_err_t nvs_flash_erase() { return ESP_FAIL; }
