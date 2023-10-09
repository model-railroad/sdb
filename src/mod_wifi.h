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

#ifndef INC_SDB_MOD_WIFI_H
#define INC_SDB_MOD_WIFI_H

// Wifi module.
// AP mode, a.k.a. "Ad-hoc wifi": this module generates its own wifi network,
//   and clients can connect to it to provide the initial configuration.
// STA mode, a.k.a. "normal wifi": this module connects to an existing wifi network,
//   provides pages for configuration, and sent its state to a JMIR or MQTT server.

#include "common.h"
#include "sdb_lock.h"
#include "sdb_mod.h"
#include "mod_display.h"

#include <algorithm>
#include <functional>
#include <vector>

#include <esp_http_server.h>
#include <Arduino_JSON.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>

#define MOD_WIFI_NAME "wi"

// This pin is checked for GND at startup, to force AP mode to reset the
// Wifi SSID + password. It\"s set to be pull-up by default.
#define FORCE_AP_PIN 36
// Defaults for AP mode. Pass must be >=7 chars.
// AP pass can be set here and this is not a secret.
#define AP_SSID "SdbNodeWifi"
#define AP_PASS "12345678"

#define AP_WIFI_ENCRYPTED   'E'
#define AP_WIFI_OPEN        'O'

#include "html/_mod_wifi_ap_index.html.gz.h"
#include "html/_mod_wifi_sta_index.html.gz.h"

class SdbModWifi : public SdbMod {
public:
    explicit SdbModWifi(SdbModManager& manager) :
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
    String _apStatus;
    wl_status_t _wifiStatus;
    // A list of SSID found when scanning. The first letter is E for encryped vs O for open.
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
        _apStatus = "Ready for configuration";

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
            bool encrypted = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
            String name(encrypted ? AP_WIFI_ENCRYPTED : AP_WIFI_OPEN);
            name += WiFi.SSID(i);
            _wifiNetworks.push_back(name);
            DEBUG_PRINTF( ( "[WIFI] scanNetworks: %d = %s\n", i, name.c_str()) );
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

        auto setLambda = [this](httpd_req_t *req) -> esp_err_t { return _setHandler(req); };
        httpd_uri_t setUri = {
            .uri = "/set",
            .method = HTTP_POST,
            .handler = &_handlerToLambda,
            .user_ctx = new std::function<esp_err_t(httpd_req_t *)>(setLambda)
        };
        httpd_register_uri_handler(httpdHandle, &setUri);
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
        // Handlers should return ESP_OK or ESP_FAIL to force closing the underlying socket.
        DEBUG_PRINTF( ( "[WIFI] _indexHandler for %p.\n", req ) );
        httpd_resp_set_type(req, "text/html");
        httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
        httpd_resp_send(req, _mod_wifi_ap_index_html_gz, _mod_wifi_ap_index_html_gz_len);
        return ESP_OK;
    }

    // Handler for /get
    esp_err_t _getHandler(httpd_req_t *req) {
        // Handlers should return ESP_OK or ESP_FAIL to force closing the underlying socket.
        DEBUG_PRINTF( ( "[WIFI] getHandler for %p.\n", req ) );
        DEBUG_PRINTF( ( "[WIFI] get uri %s.\n", req->uri ) );
        DEBUG_PRINTF( ( "[WIFI] get content_len %d.\n", req->content_len ) );
        httpd_resp_set_type(req, "application/json");

        auto ssid = _manager.dataStore().getString(SdbKey::WifiSsidStr, "");
        auto pass = _manager.dataStore().getString(SdbKey::WifiPassStr, "");

        // Note: we don't need to provide the actual password. Just the fact there's one.
        JSONVar data;
        data["id"] = ssid;
        data["pw"] = pass.isEmpty() ? "" : "•••••";
        data["st"] = _apStatus;
        int index = 0;
        for(String& n: _wifiNetworks) {
            data["ls"][index++] = n;
        }

        DEBUG_PRINTF( ( "[WIFI] get JSON %s\n", JSON.stringify(data).c_str() ) );
        httpd_resp_sendstr(req, JSON.stringify(data).c_str());
        return ESP_OK;
    }

    // Handler for /set
    esp_err_t _setHandler(httpd_req_t *req) {
        // Handlers should return ESP_OK or ESP_FAIL to force closing the underlying socket.
        DEBUG_PRINTF( ( "[WIFI] setHandler for %p.\n", req ) );
        DEBUG_PRINTF( ( "[WIFI] set uri %s.\n", req->uri ) );
        DEBUG_PRINTF( ( "[WIFI] set content_len %d.\n", req->content_len ) );

        // Constraint body content length to something reasonable
        size_t body_len = req->content_len + 1;
        if (body_len > 512) { body_len = 512; }
        std::unique_ptr<char[]> buffer(new char[body_len]);
        char* content = buffer.get();

        int ret = httpd_req_recv(req, content, body_len);
        if (ret <= 0) {
            // On success, ret is number of bytes read, > 0.
            // 0 value indicates connection closed.
            // < 0 values are error codes.>
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                httpd_resp_send_408(req);
            } else {
                httpd_resp_send_500(req);
            }
            // Nothing more can be done here.
            return ESP_FAIL;
        }

        content[body_len - 1] = 0;
        DEBUG_PRINTF( ( "[WIFI] set BODY = %s.\n", content) );
        JSONVar input = JSON.parse(content);
        String selectedSsid = input["ls"];  // or empty if not set
        String selectedPass = input["pw"];  // or empty if not set
        DEBUG_PRINTF( ( "[WIFI] Selected SSID: %s\n", selectedSsid.c_str() ));
        DEBUG_PRINTF( ( "[WIFI] Selected pass: %s\n", selectedPass.c_str() ));
        bool success = memorizeNewSsid(selectedSsid, selectedPass);

        JSONVar response;
        response["st"] = success ? "Memorized" : "Invalid Data";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, JSON.stringify(response).c_str());
        return ESP_OK;
    }

    static inline char hex2int(char c) {
        if (c >= '0' && c <= '9') return      c - '0';
        if (c >= 'A' && c <= 'F') return 10 + c - 'A';
        if (c >= 'a' && c <= 'f') return 10 + c - 'a';
        return 0;
    }

    /// Validate whether the ssid + pass is valid. If it is, memorizes the info in data store + NVS.
    /// Note that SSIDs are prefixed by either E or O (encrypted vs open).
    /// Returns true if accepted, false if not accepted.
    boolean memorizeNewSsid(const String& ssid, const String& pass) {
        // Verify ssid is one of the names we found before
        auto found = std::find(_wifiNetworks.begin(), _wifiNetworks.end(), ssid);
        if (found == std::end(_wifiNetworks)) {
            DEBUG_PRINTF( ( "[WIFI] SSID %s not found in the scanned wifi network list.\n", ssid.c_str() ));
            return false;
        }

        // Verify we have a password if one is required.
        // The "password" is encoded in a naive chr hexa pattern and should have an even length.
        if (ssid.charAt(0) == AP_WIFI_ENCRYPTED && (pass.isEmpty() || (pass.length() % 2 != 0))) {
            DEBUG_PRINTF( ( "[WIFI] SSID %s requires a non-empty password.\n", ssid.c_str() ));
            return false;
        }

        const char *pw2src = pass.c_str();
        int pwlen = pass.length() / 2;
        std::unique_ptr<char[]> buffer(new char[pwlen + 1]);
        char* pwdst = buffer.get();
        for(int i = 0; i < pwlen; i++) {
            char c = (hex2int(*pw2src++) << 4) + hex2int(*pw2src++);
            pwdst[i] = c;
        }

        pwdst[pwlen] = 0;
        String pw(pwdst);

        // Seems valid, write to the data store / NVS.
        _manager.dataStore().putString(SdbKey::WifiSsidStr, ssid);
        _manager.dataStore().putString(SdbKey::WifiPassStr, pw);
        DEBUG_PRINTF( ( "[WIFI] SSID / pass updated in data store.\n" ));
        return true;
    }
};


#endif // INC_SDB_MOD_WIFI_H
