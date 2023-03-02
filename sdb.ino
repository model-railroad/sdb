#include "src/common.h"
#include <esp_wifi.h>

#include "src/sdb_mod_manager.h"
#include "src/mod_blinky.h"
#include "src/mod_display.h"
#include "src/mod_tof.h"

SdbModManager _g_sdb_mod_manager;

void panic_blink_led() {
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

    auto blinky = new SdbModBlinky(_g_sdb_mod_manager);
    auto display = new SdbModDisplay(_g_sdb_mod_manager);
    auto tof = new SdbModTof(_g_sdb_mod_manager);
    _g_sdb_mod_manager.registerMod(blinky);
    _g_sdb_mod_manager.registerMod(display);
    _g_sdb_mod_manager.registerMod(tof);
    _g_sdb_mod_manager.onStart();
}

void loop() {
    _g_sdb_mod_manager.onLoop();
}
