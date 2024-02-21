// Test Entry Point

#include <stdio.h>

#define ESP32 1

#include "esp.h"
#include "common.h"
#include "sdb_ino.h"

// No display in the mock tests (TBD add later)
#undef USE_DISPLAY_LIB_U8G2

// These includes are specifically in the same order as the ones in sdb.ino.
// The order matters.
#include "sdb_mod_manager.h"
#include "mod_blinky.h"
#include "mod_blocks.h"
//#include "mod_display.h"
//#include "mod_jmri.h"
//#include "mod_mqtt.h"
//#include "mod_tof.h"
//#include "mod_wifi.h"

#include "sdb_lock.h"
#include "sdb_task.h"
#include "sdb_data_store.h"
#include "sdb_sensor.h"

class MainTest {
   public:
    void init() {
        printf("Hello World");
    }

};

int main() {
    MainTest test;
    test.init();
    return 0;
}
