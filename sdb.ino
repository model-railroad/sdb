#include "src/common.h"
#include <esp_wifi.h>

#include "src/sdb_mod_manager.h"
#include "src/mod_blinky.h"
#include "src/mod_display.h"
#include "src/mod_tof.h"
#include "src/mod_volt.h"
#include "src/mod_wifi.h"

SdbModManager _gSdbModManager;

void sdbPanic(char* msg) {
    if (msg != NULL) {
        ERROR_PRINTF( ("[SDB] PANIC! Cause: %s\n", msg ) );
    } else {
        ERROR_PRINTF( ("[SDB] PANIC! Please reset.\n" ) );
    }

    // Suspend task switching on *this* core.
    //
    // Note: we cannot suspend tasks on the other core. There's no API for it.
    // We could create a task with max priority on the other core and schedule it.
    //
    // Note: don't call rtDelay(), delay(), sleep(), usleep(), or Serial.printf()
    // after vTaskSuspendAll().
    vTaskSuspendAll();

    pinMode(LED_BUILTIN, OUTPUT);
    long delayMS = 150;
    bool state = false;
    while (true) {
        long endMS = millis() + delayMS;
        digitalWrite(LED_BUILTIN, state ? HIGH : LOW);
        while (millis() < endMS) {
            NOP();
        }
        state = !state;
    }
}

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    DEBUG_PRINTF( ("SDB on Core %d with priority %d, compiled using C++ %d\n",
        xPortGetCoreID(),
        uxTaskPriorityGet(NULL),
        __cplusplus) );
    DEBUG_PRINTF( ("Wifi on Core %d\n", WIFI_TASK_CORE_ID) );

    _gSdbModManager.registerMod(new SdbModBlinky(_gSdbModManager));
    _gSdbModManager.registerMod(new SdbModDisplay(_gSdbModManager));
    //_gSdbModManager.registerMod(new SdbModTof(_gSdbModManager));
    _gSdbModManager.registerMod(new SdbModVolt(_gSdbModManager));
    //_gSdbModManager.registerMod(new SdbModWifi(_gSdbModManager));
    _gSdbModManager.onStart();
}

void loop() {
    _gSdbModManager.onLoop();
}
