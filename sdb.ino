#include "src/common.h"
#include "src/sdb_mod_manager.h"
#include "src/mod_blinky.h"

SdbModManager _g_sdb_mod_manager;

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    DEBUG_PRINTF( ("SDB running on Core %d, compiled using C++ %d\n", xPortGetCoreID(), __cplusplus) );

    auto blinky = new SdbModBlinky(_g_sdb_mod_manager);
    _g_sdb_mod_manager.registerMod(blinky);
    _g_sdb_mod_manager.onStart();
}

void loop() {
    _g_sdb_mod_manager.onLoop();
}
