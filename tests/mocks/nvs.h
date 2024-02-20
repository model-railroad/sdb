#pragma once

typedef enum {
    NVS_READONLY,
    NVS_READWRITE
} nvs_open_mode_t;

namespace nvs {

    enum class ItemType : uint8_t {
        SZ,
    };

    class NVSHandle {
    public:
        esp_err_t commit() { return ESP_FAIL; }

        esp_err_t get_item_size(ItemType datatype, const char *key, size_t &size) { return ESP_FAIL; }

        template<typename T>
        esp_err_t set_item(const char *key, T value) { return ESP_FAIL; }

        esp_err_t set_string(const char *key, const char* value) { return ESP_FAIL; }

        template<typename T>
        esp_err_t get_item(const char *key, T &value) { return ESP_FAIL; }

        esp_err_t get_string(const char *key, char* out_str, size_t len) { return  ESP_FAIL; }

        esp_err_t get_blob(const char *key, void* out_blob, size_t len) { return ESP_FAIL; }

    };

    std::unique_ptr<NVSHandle> open_nvs_handle(const char *ns_name,
                                               nvs_open_mode_t open_mode,
                                               esp_err_t *err = nullptr) {
        return std::unique_ptr<NVSHandle>(new NVSHandle());
    }
}
