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
#include "sdb_block.h"
#include "mod_display.h"
#include "sdb_lock.h"
#include "sdb_mod.h"
#include "sdb_sensor.h"
#include "sdb_server.h"

#include <algorithm>
#include <functional>
#include <vector>

#include <esp_http_server.h>
#include <Arduino_JSON.h>
#include <WiFi.h>
#include <WiFiClient.h>

#define MOD_WIFI_NAME "wi"

// This pin is checked for GND at startup, to force AP mode to reset the
// Wifi SSID + password. It's set to be pull-up by default.
#define FORCE_AP_PIN 36
// Defaults for AP mode. Pass must be >=7 chars.
// AP pass can be set here and this is not a secret.
#define AP_SSID "SdbNodeWifi"
#define AP_PASS "12345678"

#define AP_WIFI_ENCRYPTED   'E'
#define AP_WIFI_OPEN        'O'

#include "html/_mod_wifi_ap_index.html.gz.h"
#include "html/_mod_wifi_sta_index.html.gz.h"
#include "html/_mod_wifi_style.css.gz.h"


enum CnxState {
    CnxIdle,
    CnxAttempt,
    CnxConnecting,
    CnxConnected,
};

class SdbModWifi : public SdbMod {
public:
    explicit SdbModWifi(SdbModManager& manager) :
        SdbMod(manager, MOD_WIFI_NAME),
       _apMode(false),
       _cnxState(CnxIdle),
       _httpdHandle(nullptr),
       _wifiStaStatus(WL_CONNECTED)    // start with an "invalid" value
    { }

    void onStart() override {
        pinMode(FORCE_AP_PIN, INPUT_PULLUP);
        selectApOrStaMode();
    }

    millis_t onLoop() override {
        if (_apMode) {
            if (_cnxState == CnxAttempt) {
                startAP();
            }
        } else {
            wl_status_t staStatus = WiFi.status();

            if (staStatus != _wifiStaStatus) {
                DEBUG_PRINTF( ("[WIFI] STA Status changed to %d\n", staStatus) );

                if (_cnxState == CnxAttempt && staStatus != WL_CONNECTED) {
                    DEBUG_PRINTF( ("[WIFI] _cnxState = StaAttempt\n") );
                    startSTA();
                } else if (_cnxState == CnxConnecting && staStatus == WL_CONNECTED) {
                    DEBUG_PRINTF( ("[WIFI] _cnxState = StaConnecting\n") );
                    connectingSTA();
                }
            }
            _wifiStaStatus = staStatus;
        }

        return _cnxState == CnxConnected ? 2000 : 500;
    }

private:
    bool _apMode;
    CnxState _cnxState;
    httpd_handle_t _httpdHandle;
    String _statusStr;
    String _staSsid;
    String _staPass;
    /// Status from WifiSTA
    wl_status_t _wifiStaStatus;
    // A list of SSID found when scanning. The first letter is E for encryped vs O for open.
    std::vector<String> _wifiNetworks;

    void selectApOrStaMode() {
        // We'll attempt to connect in AP or STA mode at the next loop.
        _cnxState = CnxAttempt;

        if (digitalRead(FORCE_AP_PIN) == LOW) {
            DEBUG_PRINTF( ( "[WIFI] Pin 36 Low ==> AP mode.\n" ) );
            _apMode = true;
            return;
        }

        const String* ssid = _manager.dataStore().getString(SdbKey::WifiSsidStr);
        const String* pass = _manager.dataStore().getString(SdbKey::WifiPassStr);
        if (ssid != nullptr) {
            _staSsid = *ssid;
        }
        if (pass != nullptr) {
            _staPass = *pass;
        }

        // AP mode if we have no SSID
        // AP mode if the SSID requires a password and we don't have one.
        if (_staSsid.isEmpty() ||
            (_staSsid.charAt(0) == AP_WIFI_ENCRYPTED && _staPass.isEmpty()) ) {
            DEBUG_PRINTF( ( "[WIFI] SSID/Pass mismatch ==> AP mode.\n" ) );
            _apMode = true;
            return;
        }

        // STA mode seems possible
        DEBUG_PRINTF( ( "[WIFI] Start in STA mode.\n" ) );
        _apMode = false;
    }

    void startAP() {
        DEBUG_PRINTF( ( "[WIFI] start AP mode.\n" ) );

        // Scanning networks forces STA mode. Do it before AP mode.
        scanNetworks();
        DEBUG_PRINTF( ( "[WIFI] End Scan Network.\n" ) );

        bool success = WiFi.softAP(AP_SSID, AP_PASS);
        if (!success) {
            ERROR_PRINTF( ( "[WIFI] AP mode did not successfully start.\n" ) );
            return;
        }

        const IPAddress ip = WiFi.softAPIP();
        DEBUG_PRINTF( ( "[WIFI] AP IP: %s.\n", ip.toString().c_str() ) );
        _manager.dataStore().putString(SdbKey::WifiApIpStr, ip.toString());

        // Display the wifi info for a few seconds, then back to sensor state.
        _manager.queueEvent(MOD_DISPLAY_NAME, SdbEvent::DisplayWifiAP);

        startAPServer();
        _statusStr = "Ready for configuration";

        _cnxState = CnxConnected;
    }

    void startSTA() {
        DEBUG_PRINTF(("[WIFI] start STA mode.\n"));

        // First character of ssid is E or O indicating encrypted vs open.
        const char *ssidptr = _staSsid.c_str();
        bool needs_password = ssidptr[0] == AP_WIFI_ENCRYPTED;

        WiFi.begin(
            ssidptr + 1,
            needs_password ? _staPass.c_str() : nullptr,
            /*channel=*/0,
            /*bssid=*/nullptr,
            /*connect=*/true);
        WiFi.setAutoReconnect(true);
        _cnxState = CnxConnecting;
    }

    void connectingSTA() {
        const IPAddress ip = WiFi.localIP();
        DEBUG_PRINTF( ( "[WIFI] STA IP: %s.\n", ip.toString().c_str() ) );
        _manager.dataStore().putString(SdbKey::WifiStaIpStr, ip.toString());

        // Display the wifi info for a few seconds, then back to sensor state.
        _manager.queueEvent(MOD_DISPLAY_NAME, SdbEvent::DisplayWifiSTA);

        startSTAServer();
        _statusStr = "Ready for serving";
        _cnxState = CnxConnected;
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

    // _handlerToLambda is a static method "trampoline" to invoke the actual handler in the
    // context of this class instance. user_ctx is expected to be the std::function to perform
    // the actual work.
    static esp_err_t _handlerToLambda(httpd_req_t *req) {
        auto lambdaPtr = static_cast< std::function<esp_err_t(httpd_req_t *)>* >(req->user_ctx);
        return (*lambdaPtr)(req);
    }

    void stopHttpdServer() {
        if (_httpdHandle != nullptr) {
            DEBUG_PRINTF( ( "[WIFI] Stop current HTTPd server\n") );
            httpd_stop(_httpdHandle);
            _httpdHandle = null;
        }
    }

    // ----- AP Server -----

    void startAPServer() {
        stopHttpdServer();

        // The ESP HTTPD server uses tasks and is all async.
        httpd_config_t httpdConfig = HTTPD_DEFAULT_CONFIG();
        httpdConfig.task_priority = SdbPriority::Network;
        httpdConfig.core_id = APP_CPU;

        // This can really only fail if the config is invalid, or if
        // task/memory cannot be allocated, which is all fatal.
        auto error = httpd_start(&_httpdHandle, &httpdConfig);
        if (error != ESP_OK) {
            PANIC_PRINTF( ( "[WIFI] httpd_start failed with error %d\n", error ) );
        }

        auto indexLambda = [this](httpd_req_t *req) -> esp_err_t { return AP_indexHandler(req); };
        httpd_uri_t indexUri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = &_handlerToLambda,
            .user_ctx = new std::function<esp_err_t(httpd_req_t *)>(indexLambda)
        };
        httpd_register_uri_handler(_httpdHandle, &indexUri);

        auto styleLambda = [this](httpd_req_t *req) -> esp_err_t { return AP_STA_styleHandler(req); };
        httpd_uri_t styleUri = {
            .uri = "/style.css",
            .method = HTTP_GET,
            .handler = &_handlerToLambda,
            .user_ctx = new std::function<esp_err_t(httpd_req_t *)>(styleLambda)
        };
        httpd_register_uri_handler(_httpdHandle, &styleUri);

        auto getLambda = [this](httpd_req_t *req) -> esp_err_t { return AP_getHandler(req); };
        httpd_uri_t getUri = {
            .uri = "/get",
            .method = HTTP_GET,
            .handler = &_handlerToLambda,
            .user_ctx = new std::function<esp_err_t(httpd_req_t *)>(getLambda)
        };
        httpd_register_uri_handler(_httpdHandle, &getUri);

        auto setLambda = [this](httpd_req_t *req) -> esp_err_t { return _setHandler(req); };
        httpd_uri_t setUri = {
            .uri = "/set",
            .method = HTTP_POST,
            .handler = &_handlerToLambda,
            .user_ctx = new std::function<esp_err_t(httpd_req_t *)>(setLambda)
        };
        httpd_register_uri_handler(_httpdHandle, &setUri);
    }

    // Handler for /
    esp_err_t AP_indexHandler(httpd_req_t *req) {
        // Handlers should return ESP_OK or ESP_FAIL to force closing the underlying socket.
        DEBUG_PRINTF( ( "[WIFI] AP_indexHandler for %p.\n", req ) );
        httpd_resp_set_type(req, "text/html");
        httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
        httpd_resp_send(req, _mod_wifi_ap_index_html_gz, _mod_wifi_ap_index_html_gz_len);
        return ESP_OK;
    }

    // Handler for /style.css
    esp_err_t AP_STA_styleHandler(httpd_req_t *req) {
        // Handlers should return ESP_OK or ESP_FAIL to force closing the underlying socket.
        DEBUG_PRINTF( ( "[WIFI] styleHandler for %p.\n", req ) );
        httpd_resp_set_type(req, "text/css");
        httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
        httpd_resp_send(req, _mod_wifi_style_html_gz, _mod_wifi_style_html_gz_len);
        return ESP_OK;
    }

    // Handler for /get
    esp_err_t AP_getHandler(httpd_req_t *req) {
        // Handlers should return ESP_OK or ESP_FAIL to force closing the underlying socket.
        DEBUG_PRINTF( ( "[WIFI] AP_getHandler for %p.\n", req ) );
        httpd_resp_set_type(req, "application/json");

        // Note: we don't need to provide the actual password. Just the fact there's one.
        JSONVar data;
        data["id"] = _staSsid;
        data["pw"] = _staPass.isEmpty() ? "" : "•••••";
        data["st"] = _statusStr;
        int index = 0;
        for(String& n: _wifiNetworks) {
            data["ls"][index++] = n;
        }

        String response = JSON.stringify(data);
        DEBUG_PRINTF( ( "[WIFI] get JSON %s\n", response.c_str() ) );
        httpd_resp_sendstr(req, response.c_str());
        return ESP_OK;
    }

    // Handler for /set
    esp_err_t _setHandler(httpd_req_t *req) {
        // Handlers should return ESP_OK or ESP_FAIL to force closing the underlying socket.
        DEBUG_PRINTF( ( "[WIFI] setHandler for %p.\n", req ) );

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
        JSONVar input = JSON.parse(content);
        String selectedSsid = input["ls"];  // or empty if not set
        String selectedPass = input["pw"];  // or empty if not set
        DEBUG_PRINTF( ( "[WIFI] Selected SSID: %s\n", selectedSsid.c_str() ));
        DEBUG_PRINTF( ( "[WIFI] Selected pass: %s\n", selectedPass.c_str() ));
        bool success = memorizeNewSsid(selectedSsid, selectedPass);

        JSONVar responseJson;
        responseJson["st"] = success ? "Memorized" : "Invalid Data";
        String response = JSON.stringify(responseJson);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, response.c_str());
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
        _staSsid = ssid;
        _staPass = pw;
        DEBUG_PRINTF( ( "[WIFI] SSID / pass updated in data store.\n" ));
        return true;
    }

    // ----- STA Server -----

    void startSTAServer() {
        stopHttpdServer();

        // The ESP HTTPD server uses tasks and is all async.
        httpd_config_t httpdConfig = HTTPD_DEFAULT_CONFIG();
        httpdConfig.task_priority = SdbPriority::Network;
        httpdConfig.core_id = APP_CPU;

        // This can really only fail if the config is invalid, or if
        // task/memory cannot be allocated, which is all fatal.
        auto error = httpd_start(&_httpdHandle, &httpdConfig);
        if (error != ESP_OK) {
            PANIC_PRINTF( ( "[WIFI] STA httpd_start failed with error %d\n", error ) );
        }

        auto indexLambda = [this](httpd_req_t *req) -> esp_err_t { return STA_indexHandler(req); };
        httpd_uri_t indexUri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = &_handlerToLambda,
            .user_ctx = new std::function<esp_err_t(httpd_req_t *)>(indexLambda)
        };
        httpd_register_uri_handler(_httpdHandle, &indexUri);

        auto styleLambda = [this](httpd_req_t *req) -> esp_err_t { return AP_STA_styleHandler(req); };
        httpd_uri_t styleUri = {
            .uri = "/style.css",
            .method = HTTP_GET,
            .handler = &_handlerToLambda,
            .user_ctx = new std::function<esp_err_t(httpd_req_t *)>(styleLambda)
        };
        httpd_register_uri_handler(_httpdHandle, &styleUri);

        auto getLambda = [this](httpd_req_t *req) -> esp_err_t { return STA_getHandler(req); };
        httpd_uri_t getUri = {
            .uri = "/get",
            .method = HTTP_GET,
            .handler = &_handlerToLambda,
            .user_ctx = new std::function<esp_err_t(httpd_req_t *)>(getLambda)
        };
        httpd_register_uri_handler(_httpdHandle, &getUri);

        auto getPropsLambda = [this](httpd_req_t *req) -> esp_err_t { return STA_getPropsHandler(req); };
        httpd_uri_t getPropsUri = {
            .uri = "/props",
            .method = HTTP_GET,
            .handler = &_handlerToLambda,
            .user_ctx = new std::function<esp_err_t(httpd_req_t *)>(getPropsLambda)
        };
        httpd_register_uri_handler(_httpdHandle, &getPropsUri);

        auto setPropsLambda = [this](httpd_req_t *req) -> esp_err_t { return STA_setPropsHandler(req); };
        httpd_uri_t setPropsUri = {
            .uri = "/props",
            .method = HTTP_POST,
            .handler = &_handlerToLambda,
            .user_ctx = new std::function<esp_err_t(httpd_req_t *)>(setPropsLambda)
        };
        httpd_register_uri_handler(_httpdHandle, &setPropsUri);
    }

    // Handler for /
    esp_err_t STA_indexHandler(httpd_req_t *req) {
        // Handlers should return ESP_OK or ESP_FAIL to force closing the underlying socket.
        DEBUG_PRINTF( ( "[WIFI] STA_indexHandler for %p.\n", req ) );
        httpd_resp_set_type(req, "text/html");
        httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
        httpd_resp_send(req, _mod_wifi_sta_index_html_gz, _mod_wifi_sta_index_html_gz_len);
        return ESP_OK;
    }

    // Handler for /get
    esp_err_t STA_getHandler(httpd_req_t *req) {
        // Handlers should return ESP_OK or ESP_FAIL to force closing the underlying socket.
        DEBUG_PRINTF( ( "[WIFI] STA_getHandler for %p.\n", req ) );
        httpd_resp_set_type(req, "application/json");

        JSONVar data = JSON.parse("{}");

        int index = 0;
        for(auto* b: _manager.blocks()) {
            data["blocks"][index++] = b->name();
        }

        index = 0;
        for(auto* s: _manager.sensors()) {
            data["sensors"][index++] = s->name();
        }

        index = 0;
        for(auto* s: _manager.servers()) {
            data["servers"][index++] = s->name();
        }

        String response = JSON.stringify(data);
        DEBUG_PRINTF( ( "[WIFI] get JSON %s\n", response.c_str() ) );
        httpd_resp_sendstr(req, response.c_str());
        return ESP_OK;
    }

    // Handler for GET /props
    esp_err_t STA_getPropsHandler(httpd_req_t *req) {
        // Handlers should return ESP_OK or ESP_FAIL to force closing the underlying socket.
        DEBUG_PRINTF( ( "[WIFI] STA_getPropsHandler for %p.\n", req ) );
        httpd_resp_set_type(req, "application/json");

        String type;
        String name;
        size_t buf_len = httpd_req_get_url_query_len(req) + 1;
        if (buf_len > 1) {
            std::unique_ptr<char[]> buffer(new char[buf_len]);
            char* query = buffer.get();
            if (httpd_req_get_url_query_str(req, query, buf_len) == ESP_OK) {
                char param[32];
                if (httpd_query_key_value(query, "t", param, sizeof(param)) == ESP_OK) {
                    type = param;
                }
                if (httpd_query_key_value(query, "n", param, sizeof(param)) == ESP_OK) {
                    name = param;
                }
            }
        }

        JSONVar data = JSON.parse("{}");

        if (type == "block") {
            for (auto *b : _manager.blocks()) {
                if (b->name() == name) {
                    JSONVar temp;
                    data["props"] = b->getProperties(temp);
                    break;
                }
            }
        } else if (type == "sensor") {
            for (auto *s : _manager.sensors()) {
                if (s->name() == name) {
                    JSONVar temp;
                    data["props"] = s->getProperties(temp);
                    break;
                }
            }
        } else if (type == "server") {
            for (auto *s : _manager.servers()) {
                if (s->name() == name) {
                    JSONVar temp;
                    data["props"] = s->getProperties(temp);
                    break;
                }
            }
        }

        String response = JSON.stringify(data);
        DEBUG_PRINTF( ( "[WIFI] get JSON '%s'\n", response.c_str() ) );
        httpd_resp_sendstr(req, response.c_str());
        return ESP_OK;
    }

    // Handler for POST /props
    esp_err_t STA_setPropsHandler(httpd_req_t *req) {
        // Handlers should return ESP_OK or ESP_FAIL to force closing the underlying socket.
        DEBUG_PRINTF( ( "[WIFI] STA_setPropsHandler for %p.\n", req ) );

        String type;
        String name;
        size_t buf_len = httpd_req_get_url_query_len(req) + 1;
        if (buf_len > 1) {
            std::unique_ptr<char[]> buffer(new char[buf_len]);
            char* query = buffer.get();
            if (httpd_req_get_url_query_str(req, query, buf_len) == ESP_OK) {
                char param[32];
                if (httpd_query_key_value(query, "t", param, sizeof(param)) == ESP_OK) {
                    type = param;
                }
                if (httpd_query_key_value(query, "n", param, sizeof(param)) == ESP_OK) {
                    name = param;
                }
            }
        }
        DEBUG_PRINTF( ( "[WIFI] set type %s --> name %s.\n", type.c_str(), name.c_str() ) );

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
        JSONVar props = input["props"];

        bool success = false;

        if (type == "block") {
            for (auto *b : _manager.blocks()) {
                if (b->name() == name) {
                    DEBUG_PRINTF( ( "[WIFI] set block %p\n", b ) );
                    b->setProperties(props);
                    success = true;
                    break;
                }
            }
        } else if (type == "sensor") {
            for (auto *s : _manager.sensors()) {
                if (s->name() == name) {
                    DEBUG_PRINTF( ( "[WIFI] set sensor %p\n", s ) );
                    s->setProperties(props);
                    success = true;
                    break;
                }
            }
        } else if (type == "server") {
            for (auto *s : _manager.servers()) {
                if (s->name() == name) {
                    DEBUG_PRINTF( ( "[WIFI] set server %p\n", s ) );
                    s->setProperties(props);
                    success = true;
                    break;
                }
            }
        }

        JSONVar responseJson;
        responseJson["st"] = success ? "Properties Saved" : "Invalid Data";
        String response = JSON.stringify(responseJson);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, response.c_str());
        return ESP_OK;
    }
};


#endif // INC_SDB_MOD_WIFI_H
