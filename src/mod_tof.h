#ifndef __INC_SDB_MOD_TOF_H
#define __INC_SDB_MOD_TOF_H

#include "common.h"
#include "sdb_mod.h"
#include <Adafruit_VL53L0X.h>
#include <Wire.h>

#define OUT_OF_RANGE_MM 2000

class SdbModTof : public SdbMod {
public:
    SdbModTof(SdbModManager& manager) :
        SdbMod(manager, "tf"),
        _tof(),
        _tof_dist_mm(OUT_OF_RANGE_MM)
    { }

    void onStart() override {
        Wire.begin(/*SDA*/ 21, /*SLC*/ 22);
        if (!_tof.begin()) {
            Serial.println(F("@@ VL53L0X begin failed (disconnected?)"));
            panic_blink_led();
        }
    }

    long onLoop() override {
        measure_tof();
        return 250;
    }

private:
    Adafruit_VL53L0X _tof;
    VL53L0X_RangingMeasurementData_t _measure;
    long _tof_dist_mm;

    void measure_tof() {
        _tof.rangingTest(&_measure, /*debug*/ false);
        
        int new_dist_mm;
        if (_measure.RangeStatus != 4) {
            new_dist_mm = _measure.RangeMilliMeter;
        } else {
            // phase failures have incorrect data
            new_dist_mm = OUT_OF_RANGE_MM;
        }
        if (_tof_dist_mm != new_dist_mm) {
            _tof_dist_mm = new_dist_mm;
            _manager.dataStore().put(SdbKey::TofDistanceMM, new_dist_mm);
        }
    }
};

#endif // __INC_SDB_MOD_TOF_H
