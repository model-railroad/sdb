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

#ifndef INC_SDB_MOD_TOF_H
#define INC_SDB_MOD_TOF_H

// Important: I2C controller 0 (aka &Wire ) is used in mod_display.
//            I2C controller 1 (aka &Wire1) is used here.
// ToF default I2C address is 0x29, pins sda=21 slc=22.

#include "common.h"
#include "sdb_lock.h"
#include "sdb_mod.h"
#include <Adafruit_VL53L0X.h>
#include <Wire.h>

#define MOD_TOF_NAME "tf"

#define OUT_OF_RANGE_MM (2 * 1000)

#define TOF_NUM 2
#define TOF_NUM_MAX 2

#define TOF0_I2C_ADDR   0x30
#define TOF1_I2C_ADDR   0x31
#define TOF0_XSHUT_PIN  18
#define TOF1_XSHUT_PIN  23

SdbKey::SdbKey ToFSdbKey[TOF_NUM_MAX] = {
    SdbKey::Tof0DistanceMmLong,
    SdbKey::Tof1DistanceMmLong
};

class SdbModTof : public SdbModTask {
public:
    explicit SdbModTof(SdbModManager& manager) :
        SdbModTask(manager, MOD_TOF_NAME, "TaskTof", SdbPriority::Sensor),
       _ioLock(_manager.ioLock())
    { }

    void onStart() override {
        Wire1.begin(/*SDA*/ 21, /*SLC*/ 22);
        init();
        for (int n = 0; n < TOF_NUM; n++) {
            _lastDistMM[n] = _manager.dataStore().putLong(ToFSdbKey[n], OUT_OF_RANGE_MM);
        }
        startTask();
    }

    long onLoop() override {
        return 2000;
    }

private:
    Adafruit_VL53L0X _tof[TOF_NUM];
    VL53L0X_RangingMeasurementData_t _measure[TOF_NUM]{};
    long _lastDistMM[TOF_NUM]{};
    SdbLock& _ioLock;

    void init() {
        pinMode(TOF0_XSHUT_PIN, OUTPUT);
        pinMode(TOF1_XSHUT_PIN, OUTPUT);

        // Reset all ToF: shutdown, wait 10ms, awake again
        digitalWrite(TOF0_XSHUT_PIN, LOW);
        digitalWrite(TOF1_XSHUT_PIN, LOW);
        delay(10 /*ms*/);
        digitalWrite(TOF0_XSHUT_PIN, HIGH);
        digitalWrite(TOF1_XSHUT_PIN, HIGH);
        delay(10 /*ms*/);

        // Activate TOF0 and keep TOF1 shutdown
        digitalWrite(TOF0_XSHUT_PIN, HIGH);
        digitalWrite(TOF1_XSHUT_PIN, LOW);
        if (!_tof[0].begin(/*i2c_addr*/ TOF0_I2C_ADDR, /*debug*/ false, /*i2c*/ &Wire1)) {
            PANIC_PRINTLN("@@ VL53L0X ToF-0 begin failed (disconnected?)");
        }
        delay(10 /*ms*/);

        if (TOF_NUM > 1) {
            // Keep TOF0 and activate TOF1
            digitalWrite(TOF1_XSHUT_PIN, HIGH);
            if (!_tof[1].begin(/*i2c_addr*/ TOF1_I2C_ADDR, /*debug*/ false, /*i2c*/ &Wire1)) {
                PANIC_PRINTLN("@@ VL53L0X ToF-1 begin failed (disconnected?)");
            }
            delay(10 /*ms*/);
        }
    }

    [[noreturn]] void onTaskRun() override {
        while(true) {
            long minDistMM = measure_tof();

            // Make refresh rate dynamic: faster when target is closer to sensor.
            long delayMS = max(50L, min(250L, minDistMM / 10));
            rtDelay(delayMS);
        }
    }

    long measure_tof() {
        {
            SdbMutex ioMutex(_ioLock);
            for (int n = 0; n < TOF_NUM; n++) {
                _tof[n].rangingTest(&_measure[n], /*debug*/ false);
            }
        }

        long minDistMM = OUT_OF_RANGE_MM;
        for (int n = 0; n < TOF_NUM; n++) {
            int newDistMM;
            if (_measure[n].RangeStatus != 4) {
                newDistMM = _measure[n].RangeMilliMeter;
            } else {
                // phase failures have incorrect data
                newDistMM = OUT_OF_RANGE_MM;
            }
            minDistMM = MIN(minDistMM, newDistMM);

            if (_lastDistMM[n] != newDistMM) {
                _lastDistMM[n] = _manager.dataStore().putLong(ToFSdbKey[n], newDistMM);
            }
        }

        return minDistMM;
    }
};

#endif // INC_SDB_MOD_TOF_H
