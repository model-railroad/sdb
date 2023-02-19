#ifndef __INC_SDB_MOD_BLINKY_H
#define __INC_SDB_MOD_BLINKY_H

#include "common.h"
#include "sdb_mod.h"

class SdbModBlinky : public SdbMod {
public:
    SdbModBlinky(SdbModManager& manager) :
        SdbMod(manager, "ld") {
    }

    void onStart() override {
        pinMode(BUILTIN_LED, OUTPUT);
        pinMode(19, OUTPUT);
    }
    
    int onLoop() override {
        digitalWrite(BUILTIN_LED, HIGH);
        digitalWrite(19, HIGH);
        delay(250 /*ms*/);
        digitalWrite(BUILTIN_LED, LOW);
        digitalWrite(19, LOW);
        // delay(1000 /*ms*/);
        return 1000;
    }

private:
    //
};

#endif // __INC_SDB_MOD_BLINKY_H
