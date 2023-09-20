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

#define MOD_TOF_NAME "tf"

class SdbModTof : public SdbModTask {
public:
    SdbModTof(SdbModManager& manager) :
        SdbModTask(manager, MOD_TOF_NAME, "TaskTof", SdbPriority::Sensor),
        _ioLock(_manager.ioLock()),
        _dataLock(_manager.dataStore().lock()),
        _tof(),
        _sharedDistMM(NULL)
    { }

    void onStart() override {
        Wire1.begin(/*SDA*/ 21, /*SLC*/ 22);
        if (!_tof.begin(/*i2c_addr*/ VL53L0X_I2C_ADDR, /*debug*/ false, /*i2c*/ &Wire1)) {
            Serial.println(F("@@ VL53L0X begin failed (disconnected?)"));
            sdbPanic();
        }
        _sharedDistMM = _manager.dataStore().ptrLong(SdbKey::TofDistanceMmLong, OUT_OF_RANGE_MM);
        startTask();
    }

    long onLoop() override {
        return 2000;
    }

private:
    Adafruit_VL53L0X _tof;
    VL53L0X_RangingMeasurementData_t _measure;
    long* _sharedDistMM;
    SdbLock& _ioLock;
    SdbLock& _dataLock;

    void onTaskRun() {
        while(true) {
            long distMM = measure_tof();
            update_data_store(distMM);

            // Make refresh rate dynamic: faster when target is closer to sensor.
            long delayMS = max(50L, min(250L, distMM / 10));
            rtDelay(delayMS);
        }
    }

    long measure_tof() {
        {
            SdbMutex ioMutex(_ioLock);
            _tof.rangingTest(&_measure, /*debug*/ false);
        }
        
        int newDistMM;
        if (_measure.RangeStatus != 4) {
            newDistMM = _measure.RangeMilliMeter;
            // DEBUG_PRINTF( ("[TOF] dist %ld mm\n", newDistMM) );
        } else {
            // phase failures have incorrect data
            newDistMM = OUT_OF_RANGE_MM;
        }

        return newDistMM;
    }

    void update_data_store(long newDistMM) {
        SdbMutex dataMutex(_dataLock);
        if (*_sharedDistMM != newDistMM) {
            *_sharedDistMM = newDistMM;
        }
    }
};

#endif // __INC_SDB_MOD_TOF_H
