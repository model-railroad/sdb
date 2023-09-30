/*
 * Project: Software Defined Blocks
 * Copyright (C) 2023 alf.labs gmail com.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __INC_SDB_MOD_WIFI_H
#define __INC_SDB_MOD_WIFI_H

// Wifi module.
// AP mode, a.k.a. "Ad-hoc wifi": this module generates its own wifi network,
//   and clients can connect to it to provide the initial configuration.
// STA mode, a.k.a. "normal wifi": this module connects to an existing wifi network,
//   provides pages for configuration, and sent its state to a JMIR or MQTT server.

#include "common.h"
#include "sdb_lock.h"
#include "sdb_mod.h"
#include "mod_display.h"
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <esp_http_server.h>
#include <functional>
#include <vector>

// This pin is checked for GND at startup, to force AP mode to reset the
// Wifi SSID + password. It's set to be pull-up by default.
#define FORCE_AP_PIN 36
// Defaults for AP mode. Pass must be >=7 chars.
// AP pass can be set here and this is not a secret.
#define AP_SSID "SdbNodeWifi"
#define AP_PASS "12345678"

#define MOD_WIFI_NAME "wi"

#include "html/_mod_wifi_ap_index.html.gz.h"
#include "html/_mod_wifi_sta_index.html.gz.h"

class SdbModWifi : public SdbMod {
public:
    SdbModWifi(SdbModManager& manager) :
        SdbMod(manager, MOD_WIFI_NAME),
        _apMode(false),
        _wifiStatus(WL_NO_SHIELD)    // start with an "invalid" value
    { }

    void onStart() override {
        pinMode(FORCE_AP_PIN, INPUT_PULLUP);

        _apMode = checkApMode();
        bool started = _apMode ? startAP() : startSTA();
        if (!started) {
            PANIC_PRINTF( ( "[WIFI] wifi did not start\n" ) );
        }
    }

    long onLoop() override {
        wl_status_t status = WiFi.status();
        if (status != _wifiStatus) {
            _wifiStatus = status;
            DEBUG_PRINTF( ("[WIFI] Status changed to %d\n", status) );
        }
        return 2000;
    }

private:
    bool _apMode;
    wl_status_t _wifiStatus;
    std::vector<String> _wifiNetworks;

    bool checkApMode() {
        if (digitalRead(FORCE_AP_PIN) == LOW) {
            return true;
        }

        // TBD for now only has AP mode
        return true;
    }

    bool startAP() {
        DEBUG_PRINTF( ( "[WIFI] AP mode enabled.\n" ) );

        // Scanning networks forces STA mode. Do it before AP mode.
        scanNetworks();

        bool success = WiFi.softAP(AP_SSID, AP_PASS);
        if (!success) {
            ERROR_PRINTF( ( "[WIFI] AP mode did not successfully start.\n" ) );
            return false;
        }

        const IPAddress ip = WiFi.softAPIP();
        DEBUG_PRINTF( ( "[WIFI] AP IP: %s.\n", ip.toString().c_str() ) );
        _manager.dataStore().putString(SdbKey::SoftApIpStr, ip.toString());

        // Display the wifi info for a few seconds, then back to sensor state.
        _manager.queueEvent(MOD_DISPLAY_NAME, SdbEvent::DisplayWifiAP);

        startAPServer();

        return true;
    }

    bool startSTA() {
        ERROR_PRINTF( ( "[WIFI] STA mode not implemented yet.\n" ) );
        _manager.queueEvent(MOD_DISPLAY_NAME, SdbEvent::DisplaySensor);
        return false;
    }

    void scanNetworks() {
        // Note: in sync mode, implementation has a 10-second timeout.
        // This also temporarily changes the mode to STA.
        int n = WiFi.scanNetworks(/*async*/ false, /*show_hidden*/ false);
        DEBUG_PRINTF( ( "[WIFI] scanNetworks: found %d networks.\n", n ) );
        for (int i = 0; i < n; ++i) {
            _wifiNetworks.push_back(WiFi.SSID(i));
            DEBUG_PRINTF( ( "[WIFI] scanNetworks: %d = %s\n", i, WiFi.SSID(i).c_str()) );
        }
        WiFi.scanDelete(); // free memory
    }

    void startAPServer() {
        // The ESP HTTPD server uses tasks and is all async.
        httpd_handle_t httpdHandle = NULL;
        httpd_config_t httpdConfig = HTTPD_DEFAULT_CONFIG();
        httpdConfig.task_priority = SdbPriority::Network;
        httpdConfig.core_id = APP_CPU;

        // This can really only fail if the config is invalid, or if
        // task/memory cannot be allocated, which is all fatal.
        auto error = httpd_start(&httpdHandle, &httpdConfig);
        if (error != ESP_OK) {
            PANIC_PRINTF( ( "[WIFI] httpd_start failed with error %d\n", error ) );
        }

        auto indexLambda = [this](httpd_req_t *req) -> esp_err_t { return _indexHandler(req); };
        httpd_uri_t indexUri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = &_handlerToLambda,
            .user_ctx = new std::function<esp_err_t(httpd_req_t *)>(indexLambda)
        };
        httpd_register_uri_handler(httpdHandle, &indexUri);

        auto getLambda = [this](httpd_req_t *req) -> esp_err_t { return _getHandler(req); };
        httpd_uri_t getUri = {
            .uri = "/get",
            .method = HTTP_GET,
            .handler = &_handlerToLambda,
            .user_ctx = new std::function<esp_err_t(httpd_req_t *)>(getLambda)
        };
        httpd_register_uri_handler(httpdHandle, &getUri);
    }

    // _handlerToLambda is a static method "trampoline" to invoke the actual handler in the
    // context of this class instance. user_ctx is expected to be the std::function to perform
    // the actual work.
    static esp_err_t _handlerToLambda(httpd_req_t *req) {
        DEBUG_PRINTF( ( "[WIFI] _handlerLambda for %p -> %p.\n", req, req->user_ctx ) );
        auto lambdaPtr = static_cast< std::function<esp_err_t(httpd_req_t *)>* >(req->user_ctx);
        return (*lambdaPtr)(req);
    }

    // Handler for /
    esp_err_t _indexHandler(httpd_req_t *req) {
        DEBUG_PRINTF( ( "[WIFI] _indexHandler for %p.\n", req ) );
        httpd_resp_set_type(req, "text/html");
        httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
        httpd_resp_send(req, _mod_wifi_ap_index_html_gz, _mod_wifi_ap_index_html_gz_len);
        return ESP_OK;
    }

    // Handler for /get
    esp_err_t _getHandler(httpd_req_t *req) {
        DEBUG_PRINTF( ( "[WIFI] getHandler for %p.\n", req ) );
        httpd_resp_set_type(req, "text/json");
        httpd_resp_sendstr(req, "response");
        return ESP_OK;
    }
};


#endif // __INC_SDB_MOD_WIFI_H
