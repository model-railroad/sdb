#pragma once

#include <WString.h>

typedef enum {
    WL_NO_SHIELD        = 255,
    WL_IDLE_STATUS      = 0,
    WL_NO_SSID_AVAIL    = 1,
    WL_SCAN_COMPLETED   = 2,
    WL_CONNECTED        = 3,
    WL_CONNECT_FAILED   = 4,
    WL_CONNECTION_LOST  = 5,
    WL_DISCONNECTED     = 6
} wl_status_t;

typedef enum {
    WIFI_AUTH_OPEN = 0,         /**< authenticate mode : open */
    WIFI_AUTH_WEP,              /**< authenticate mode : WEP */
    WIFI_AUTH_WPA_PSK,          /**< authenticate mode : WPA_PSK */
    WIFI_AUTH_WPA2_PSK,         /**< authenticate mode : WPA2_PSK */
    WIFI_AUTH_WPA_WPA2_PSK,     /**< authenticate mode : WPA_WPA2_PSK */
    WIFI_AUTH_WPA2_ENTERPRISE,  /**< authenticate mode : WPA2_ENTERPRISE */
    WIFI_AUTH_WPA3_PSK,         /**< authenticate mode : WPA3_PSK */
    WIFI_AUTH_WPA2_WPA3_PSK,    /**< authenticate mode : WPA2_WPA3_PSK */
    WIFI_AUTH_WAPI_PSK,         /**< authenticate mode : WAPI_PSK */
    WIFI_AUTH_MAX
} wifi_auth_mode_t;

#define WIFI_SCAN_RUNNING   (-1)
#define WIFI_SCAN_FAILED    (-2)

class IPAddress {
public:

    String toString() const {
        return "127.0.0.42";
    }
};

class WiFiClass {
public:

    // ----- WiFi Scan -----

    int16_t scanNetworks(bool async = false, bool show_hidden = false, bool passive = false, uint32_t max_ms_per_chan = 300, uint8_t channel = 0, const char * ssid=nullptr, const uint8_t * bssid=nullptr) {
        return WIFI_SCAN_FAILED;
    }

    wifi_auth_mode_t encryptionType(uint8_t i) {
        return WIFI_AUTH_OPEN;
    }

    String SSID(uint8_t i) {
        return "MySSID";
    }

    void scanDelete() {}

    // ----- WiFi STA -----

    wl_status_t status() { return WL_DISCONNECTED; }

    wl_status_t begin(const char* ssid, const char *passphrase, int32_t channel, const uint8_t* bssid, bool connect) {
        return WL_CONNECT_FAILED;
    }

    bool setAutoReconnect(bool autoReconnect) {
        return false;
    }

    IPAddress localIP() { return IPAddress(); }


    // ----- WiFi AP -----

    bool softAP(const char* ssid, const char* passphrase = NULL, int channel = 1, int ssid_hidden = 0, int max_connection = 4, bool ftm_responder = false) {
        return false;
    }

    IPAddress softAPIP() { return IPAddress(); }

};

WiFiClass WiFi;
