#ifndef __INC_SDB_MOD_WIFI_H
#define __INC_SDB_MOD_WIFI_H

#include "common.h"
#include "sdb_lock.h"
#include "sdb_mod.h"
#include "mod_display.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <vector>

// This pin is checked for GND at startup, to force AP mode to reset the
// Wifi SSID + password. It's set to be pull-up by default.
#define FORCE_AP_PIN 36
// Defaults for AP mode. Pass must be >=7 chars.
// AP pass can be set here and this is not a secret.
#define AP_SSID "SdbNodeWifi"
#define AP_PASS "12345678"

#define MOD_WIFI_NAME "wi"

class SdbModWifi : public SdbModTask {
public:
    SdbModWifi(SdbModManager& manager) :
        SdbModTask(manager, MOD_WIFI_NAME, "TaskWifi", SdbPriority::Network),
        _apMode(false),
        _wifiStatus(WL_NO_SHIELD)    // start with an "invalid" value
    { }

    void onStart() override {
        pinMode(FORCE_AP_PIN, INPUT_PULLUP);

        _apMode = checkApMode();
        bool started = _apMode ? startAP() : startSTA();
        if (started) {
            startTask();
        } else {
            PANIC_PRINTF( ( "[WIFI] wifi did not start -- TBD move in loop & retry\n" ) );
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
        }

        const IPAddress ip = WiFi.softAPIP();
        DEBUG_PRINTF( ( "[WIFI] AP IP: %s.\n", ip.toString().c_str() ) );
        _manager.dataStore().putString(SdbKey::SoftAPIP, ip.toString());
        _manager.queueEvent(MOD_DISPLAY_NAME, SdbEvent::DisplayWifiAP);

        return success;
    }

    bool startSTA() {
        ERROR_PRINTF( ( "[WIFI] STA mode not implemented yet.\n" ) );
        _manager.queueEvent(MOD_DISPLAY_NAME, SdbEvent::DisplaySensor);
        return false;
    }

    void onTaskRun() override {
        // TBD do init stuff

        while (true) {
            // TBD check http server
            rtDelay(250 /*ms*/);
        }
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
};


#endif // __INC_SDB_MOD_WIFI_H
