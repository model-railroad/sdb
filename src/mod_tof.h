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
#include "sdb_props.h"
#include "sdb_sensor.h"

#include <Adafruit_VL53L0X.h>
#include <Arduino_JSON.h>
#include <Wire.h>

#define MOD_TOF_NAME "tf"

#define OUT_OF_RANGE_MM (2 * 1000)

#define TOF_NUM 1

// Default address when the ToF sensor does not have XShut pin connected.
#define TOF_DEFAULT_I2C_ADDR   0x29

// Define these if the ToF sensors have their XShut pin connected.
#define TOF0_I2C_ADDR   0x30
#if TOF_NUM > 1
#define TOF1_I2C_ADDR   0x31
#endif
#define TOF0_XSHUT_PIN  18
#define TOF1_XSHUT_PIN  23

//-----------------------------------

class SdbSensorTof;
static ISdbSensorBlockLogic* makeSdbSensorBlockLogicTof(
    SdbModManager& manager, SdbSensorTof* sensor, int blockIndex);

class SdbSensorTof : public SdbSensor {
public:
    SdbSensorTof(SdbModManager& manager, String&& name, uint8_t i2cAddr) :
       SdbSensor(manager, std::forward<String>(name)),
       _i2cAddr(i2cAddr),
       _lastDistMM(OUT_OF_RANGE_MM)
    { }

    ISdbSensorBlockLogic* makeLogic(int blockIndex) override {
        return makeSdbSensorBlockLogicTof(_manager, this, blockIndex);
    }

    void init() {
        if (!_tof.begin(_i2cAddr, /*debug*/ false, /*i2c*/ &Wire1)) {
            PANIC_PRINTF( ("@@ VL53L0X ToF %s begin failed (disconnected?)", name().c_str()) );
        }
    }

    /// Performs a Ranging Test; uses I2C. Should be wrapped in an IOLock mutex.
    void rangingTest() {
        _tof.rangingTest(&_measure, /*debug*/ false);
    }

    /// Converts the result of the last rangingTest() call into a distance.
    /// Updates _lastDistMM and returns that value.
    int measure() {
        int newDistMM;
        if (_measure.RangeStatus != 4) {
            newDistMM = _measure.RangeMilliMeter;
        } else {
            // phase failures have incorrect data
            newDistMM = OUT_OF_RANGE_MM;
        }

        _lastDistMM = newDistMM;
        return newDistMM;
    }

    /// Returns the last distance computed by measure().
    long lastDistMM() const {
        return _lastDistMM;
    }

#if defined(USE_DISPLAY_LIB_U8G2)
    void draw(U8G2_SSD1306_128X64_NONAME_F_HW_I2C& _u8g2, int y) override {
        long value = _lastDistMM;
        String dt = String(value);
        _u8g2.drawStr(0, y, dt.c_str());

        // Frame is an empty Box. Box is filled.
        _u8g2.drawFrame(64, y, 128, 8);
        float w = 64.0f / 2000.0f * value;
        _u8g2.drawBox(64, y, min(64, max(0, (int)w)), 8);
    }
#endif

    /// Get current properties and fill in JSON var.
    JSONVar& getProperties(JSONVar &output) override {
        JSONVar temp;

        output["sr:name.s" ] = mkProp(temp, "Name",                 name());
        output["sr:desc.s" ] = mkProp(temp, "Description",          "Adafruit VL53L0X ToF");
        output["sr!value.i"] = mkProp(temp, "Distance (mm)",        String(_lastDistMM));

        return output;
    }

    /// Store new properties provided by the JSON var.
    void setProperties(JSONVar &input) override {
        // no-op
    }

private:
    uint8_t _i2cAddr;
    Adafruit_VL53L0X _tof;
    VL53L0X_RangingMeasurementData_t _measure{};
    long _lastDistMM;
};

//-----------------------------------

class SdbSensorBlockLogicTof : public SdbSensorBlockLogic<SdbSensorTof> {
   public:
    SdbSensorBlockLogicTof(SdbModManager& manager, SdbSensorTof* sensor,
                           int blockIndex) :
          SdbSensorBlockLogic(manager, sensor),
          _keyMinLong(static_cast<SdbKey::SdbKey>(SdbKey::Tof0MinMmLong + blockIndex)),
          _keyMaxLong(static_cast<SdbKey::SdbKey>(SdbKey::Tof0MaxMmLong + blockIndex))
    { }

    void init() {
        _minThreshold = _manager.dataStore().getLong(_keyMinLong, 0);
        _maxThreshold = _manager.dataStore().getLong(_keyMaxLong, OUT_OF_RANGE_MM);
    }

    bool state() const override {
        return _minThreshold <= _sensor->lastDistMM() && _sensor->lastDistMM() <= _maxThreshold;
    }

    /// Read current properties and fill in JSON var.
    JSONVar& getProperties(JSONVar &output) override {
        JSONVar temp;

        output["sr.min.i"  ] = mkProp(temp, "Min Threshold (mm)",   String(_minThreshold));
        output["sr.max.i"  ] = mkProp(temp, "Max Threshold (mm)",   String(_maxThreshold));

        return output;
    }

    /// Parse JSON var and store new mutable properties. Ignore non-mutable properties.
    void setProperties(JSONVar &input) override {
        String newMin = input["sr.min.i"];      // empty if not set
        String newMax = input["sr.max.i"];      // empty if not set

        newMin.trim();
        newMax.trim();

        if (!newMin.isEmpty()) {
            _minThreshold = newMin.toInt();
            _manager.dataStore().putLong(_keyMinLong, _minThreshold);
        }
        if (!newMax.isEmpty()) {
            _maxThreshold = newMax.toInt();
            _manager.dataStore().putLong(_keyMaxLong, _maxThreshold);
        }
    }

private:
    long _minThreshold;
    long _maxThreshold;
    SdbKey::SdbKey _keyMinLong;
    SdbKey::SdbKey _keyMaxLong;
};


static ISdbSensorBlockLogic* makeSdbSensorBlockLogicTof(
    SdbModManager& manager, SdbSensorTof* sensor, int blockIndex)  {
    return new SdbSensorBlockLogicTof(manager, sensor, blockIndex);
}


//-----------------------------------

class SdbModTof : public SdbModTask {
public:
    explicit SdbModTof(SdbModManager& manager) :
        SdbModTask(manager, MOD_TOF_NAME, "TaskTof", SdbPriority::Sensor),
       _ioLock(_manager.ioLock()),
       _tof{
#ifdef TOF0_I2C_ADDR
           {manager,
            "tof0",
            TOF0_I2C_ADDR},
#else
          {manager, "tof0",
           TOF_DEFAULT_I2C_ADDR},
#endif
#ifdef TOF1_I2C_ADDR
           {manager,
            "tof1",
            TOF1_I2C_ADDR}
#endif
       }
    { }

    void onStart() override {
        Wire1.begin(/*SDA*/ 21, /*SLC*/ 22);
        init();

        for(auto& t: _tof) {
            _manager.registerSensor(t);
        }

        startTask();
    }

    millis_t onLoop() override {
        return 2000;
    }

private:
    SdbSensorTof _tof[TOF_NUM];
    SdbLock& _ioLock;

    void init() {
#ifdef TOF0_XSHUT_PIN
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
        _tof[0].init();
        delay(10 /*ms*/);

    #if TOF_NUM > 1
        // Keep TOF0 and activate TOF1
        digitalWrite(TOF1_XSHUT_PIN, HIGH);
        _tof[1].init();
        delay(10 /*ms*/);
    #endif
#endif
    }

    [[noreturn]] void onTaskRun() override {
        while(true) {
            long minDistMM = measure_tof();

            // Make refresh rate dynamic: faster when target is closer to sensor.
            millis_t delayMS = std::max(50L, std::min(250L, minDistMM / 10));
            rtDelay(delayMS);
        }
    }

    long measure_tof() {
        for (auto& tof : _tof) {
            SdbMutex ioMutex(_ioLock);
            tof.rangingTest();
        }

        long minDistMM = OUT_OF_RANGE_MM;
        for (auto& tof : _tof) {
            int newDistMM = tof.measure();
            minDistMM = MIN(minDistMM, newDistMM);
        }

        // STAMeasureFail is not used since the ToF reading never really "fails" as it times out
        // if there's nothing to detect. The only real failure is some kind of I2C error which we
        // don't really catch here anyway.
        BLINK_EVENT(_manager, SdbBlinkMode::STAMeasureOK);

        return minDistMM;
    }
};

#endif // INC_SDB_MOD_TOF_H
