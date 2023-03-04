#ifndef __INC_SDB_MOD_TOF_H
#define __INC_SDB_MOD_TOF_H

// Important: I2C controller 0 (aka &Wire ) is used in mod_display.
//            I2C controller 1 (aka &Wire1) is used here.
// ToF default I2C address is 0x29, pins sda=21 slc=22.

#include "common.h"
#include "sdb_lock.h"
#include "sdb_mod.h"
#include <Adafruit_VL53L0X.h>
#include <Wire.h>

#define OUT_OF_RANGE_MM 2*1000

class SdbModTof : public SdbModTask {
public:
    SdbModTof(SdbModManager& manager) :
        SdbModTask(manager, "tf", "TaskTof", SdbPriority::Sensor),
        _io_lock(_manager.ioLock()),
        _data_lock(_manager.dataStore().lock()),
        _tof(),
        _exported_dist_mm(NULL)
    { }

    void onStart() override {
        Wire1.begin(/*SDA*/ 21, /*SLC*/ 22);
        if (!_tof.begin(/*i2c_addr*/ VL53L0X_I2C_ADDR, /*debug*/ false, /*i2c*/ &Wire1)) {
            Serial.println(F("@@ VL53L0X begin failed (disconnected?)"));
            sdb_panic();
        }
        _exported_dist_mm = _manager.dataStore().ptrLong(SdbKey::TofDistanceMM, OUT_OF_RANGE_MM);
        startTask();
    }

    long onLoop() override {
        return 2000;
    }

private:
    Adafruit_VL53L0X _tof;
    VL53L0X_RangingMeasurementData_t _measure;
    long* _exported_dist_mm;
    SdbLock& _io_lock;
    SdbLock& _data_lock;

    void onTaskRun() {
        while(true) {
            long dist_mm = measure_tof();
            update_data_store(dist_mm);

            // Make refresh rate dynamic: faster when target is closer to sensor.
            long delay_ms = max(50L, min(250L, dist_mm / 10));
            rtDelay(delay_ms);
        }
    }

    long measure_tof() {
        {
            SdbMutex io_mutex(_io_lock);
            _tof.rangingTest(&_measure, /*debug*/ false);
        }
        
        int new_dist_mm;
        if (_measure.RangeStatus != 4) {
            new_dist_mm = _measure.RangeMilliMeter;
        } else {
            // phase failures have incorrect data
            new_dist_mm = OUT_OF_RANGE_MM;
        }

        return new_dist_mm;
    }

    void update_data_store(long new_dist_mm) {
        SdbMutex data_mutex(_data_lock);
        if (*_exported_dist_mm != new_dist_mm) {
            *_exported_dist_mm = new_dist_mm;
        }
    }
};

#endif // __INC_SDB_MOD_TOF_H
