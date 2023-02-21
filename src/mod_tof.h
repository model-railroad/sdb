#ifndef __INC_SDB_MOD_TOF_H
#define __INC_SDB_MOD_TOF_H

#include "common.h"
#include "sdb_mod.h"
#include <Adafruit_VL53L0X.h>
#include <Wire.h>

class SdbModTof : public SdbMod {
public:
    SdbModTof(SdbModManager& manager) :
        SdbMod(manager, "tf"),
        _tof()
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
    uint16_t _tof_dist_mm = 0;
    
    #define YTXT 22

    void measure_tof() {
        Serial.println("@@ Loop TOP");
        _tof.rangingTest(&_measure, /*debug*/ true);
        _tof_dist_mm = _measure.RangeMilliMeter;
        
        if (_measure.RangeStatus != 4) {  // phase failures have incorrect data
            Serial.print("@@ TOF distance mm: "); 
            Serial.println(_tof_dist_mm);
            temp_global_dist = _tof_dist_mm;

//            digitalWrite(PIN_LED2, _tof_dist_mm < THRESHOLD_MM ? HIGH : LOW);

        } else {
            Serial.println("@@ TOF out of range ");
//            digitalWrite(PIN_LED2, LOW);
        }
    }
};

#endif // __INC_SDB_MOD_TOF_H
