#include "mod_tof.h"

void SdbModTof::onStart() {
    Wire1.begin(/*SDA*/ 21, /*SLC*/ 22);
    if (!_tof.begin(/*i2c_addr*/ VL53L0X_I2C_ADDR, /*debug*/ false, /*i2c*/ &Wire1)) {
        Serial.println(F("@@ VL53L0X begin failed (disconnected?)"));
        sdbPanic();
    }
    _sharedDistMM = _manager.dataStore().ptrLong(SdbKey::TofDistanceMM, OUT_OF_RANGE_MM);
    startTask();
}

long SdbModTof::onLoop() {
    return 2000;
}

void SdbModTof::onTaskRun() {
    while(true) {
        long distMM = measure_tof();
        update_data_store(distMM);

        // Make refresh rate dynamic: faster when target is closer to sensor.
        long delayMS = max(50L, min(250L, distMM / 10));
        rtDelay(delayMS);
    }
}

long SdbModTof::measure_tof() {
    {
        SdbMutex ioMutex(_ioLock);
        _tof.rangingTest(&_measure, /*debug*/ false);
    }
    
    int newDistMM;
    if (_measure.RangeStatus != 4) {
        newDistMM = _measure.RangeMilliMeter;
    } else {
        // phase failures have incorrect data
        newDistMM = OUT_OF_RANGE_MM;
    }

    return newDistMM;
}

void SdbModTof::update_data_store(long newDistMM) {
    SdbMutex dataMutex(_dataLock);
    if (*_sharedDistMM != newDistMM) {
        *_sharedDistMM = newDistMM;
    }
}

