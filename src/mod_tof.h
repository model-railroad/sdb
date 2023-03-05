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
        _ioLock(_manager.ioLock()),
        _dataLock(_manager.dataStore().lock()),
        _tof(),
        _sharedDistMM(NULL)
    { }

    void onStart() override;

    long onLoop() override;

private:
    Adafruit_VL53L0X _tof;
    VL53L0X_RangingMeasurementData_t _measure;
    long* _sharedDistMM;
    SdbLock& _ioLock;
    SdbLock& _dataLock;

    void onTaskRun();

    long measure_tof();

    void update_data_store(long newDistMM);
};

#endif // __INC_SDB_MOD_TOF_H
