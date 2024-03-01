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

#include "common.h"
#include <esp_wifi.h>

#include "sdb_mod_manager.h"
#include "mod_blinky.h"
#include "mod_blocks.h"
#include "mod_display.h"
#include "mod_jmri.h"
#include "mod_mqtt.h"
#include "mod_tof.h"
#include "mod_wifi.h"

SdbModManager _gSdbModManager;

[[noreturn]] void sdbPanic(char* msg) {
    if (msg != nullptr) {
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

void _gSdbSetup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    DEBUG_PRINTF( ("SDB on Core %d with priority %d, compiled using C++ %d\n",
        xPortGetCoreID(),
        uxTaskPriorityGet(nullptr),
        __cplusplus) );
    DEBUG_PRINTF( ("Wifi on Core %d\n", WIFI_TASK_CORE_ID) );

    auto blocks  = new SdbModBlocks(_gSdbModManager);
    auto blinky  = new SdbModBlinky(_gSdbModManager);
    auto display = new SdbModDisplay(_gSdbModManager);
    auto jmri    = new SdbModJmri(_gSdbModManager);
    auto mqtt    = new SdbModMqtt(_gSdbModManager);
    auto tof     = new SdbModTof(_gSdbModManager);
    auto wifi    = new SdbModWifi(_gSdbModManager);
    // Note: order of registration dictates order of execution for onStart() and onLoop().
    _gSdbModManager.registerMod(blinky);
    _gSdbModManager.registerMod(display);
    _gSdbModManager.registerMod(tof);
    _gSdbModManager.registerMod(jmri);
    _gSdbModManager.registerMod(mqtt);
    _gSdbModManager.registerMod(blocks); // after sensors modules
    _gSdbModManager.registerMod(wifi);
    _gSdbModManager.onStart();
}

void _gSdbLoop() {
    _gSdbModManager.onLoop();
}
