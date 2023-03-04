#include "src/common.h"
#include <esp_wifi.h>

#include "src/sdb_mod_manager.h"
#include "src/mod_blinky.h"
#include "src/mod_display.h"
#include "src/mod_tof.h"

SdbModManager _gSdbModManager;

void sdbPanic(char* msg) {
    if (msg != NULL) {
        ERROR_PRINTF( ("[SDB] PANIC! Cause: %s\n", msg ) );
    }
    pinMode(LED_BUILTIN, OUTPUT);
    while (true) {
        digitalWrite(LED_BUILTIN, LOW);
        delay(500 /*ms*/);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500 /*ms*/);
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

    auto blinky = new SdbModBlinky(_gSdbModManager);
    auto display = new SdbModDisplay(_gSdbModManager);
    auto tof = new SdbModTof(_gSdbModManager);
    _gSdbModManager.registerMod(blinky);
    _gSdbModManager.registerMod(display);
    _gSdbModManager.registerMod(tof);
    _gSdbModManager.onStart();
}

void loop() {
    _gSdbModManager.onLoop();
}
