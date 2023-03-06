#ifndef __INC_SDB_MOD_WIFI_H
#define __INC_SDB_MOD_WIFI_H

#include "common.h"
#include "sdb_lock.h"
#include "sdb_mod.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

// This pin is checked for GND at startup, to force AP mode to reset the
// Wifi SSID + password. It's set to be pull-up by default.
#define FORCE_AP_PIN 36


class SdbModWifi : public SdbModTask {
public:
    SdbModWifi(SdbModManager& manager) :
        SdbModTask(manager, "wi", "TaskWifi", SdbPriority::Network),
        apMode(false)
    { }

    void onStart() override {
        pinMode(FORCE_AP_PIN, INPUT_PULLUP);

        apMode = checkApMode();
        bool started = apMode ? startAP() : startSTA();
        if (started) {
            startTask();
        } else {
            PANIC_PRINTF( ( "[WIFI] wifi did not start -- TBD move in loop & retry\n" ) );
        }
    }

    long onLoop() override {
        return 2000;
    }

private:
    bool apMode;

    bool checkApMode() {
        if (digitalRead(FORCE_AP_PIN) == LOW) {
            return true;
        }

        // TBD for now only has AP mode
        return true;
    }

    bool startAP() {
        DEBUG_PRINTF( ( "[WIFI] AP mode enabled.\n" ) );
        ERROR_PRINTF( ( "[WIFI] AP mode did not successfully start.\n" ) );
        return false;
    }

    bool startSTA() {
        ERROR_PRINTF( ( "[WIFI] STA mode not implemented yet.\n" ) );
        return false;
    }

    void onTaskRun() override {
        // TBD do init stuff

        while (true) {
            // TBD check http server
            rtDelay(250 /*ms*/);
        }
    }
};


#endif // __INC_SDB_MOD_WIFI_H
