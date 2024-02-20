#pragma once

#include <Wire.h>

typedef int8_t VL53L0X_Error;

typedef struct {
    uint16_t RangeMilliMeter;
    uint8_t RangeStatus;
} VL53L0X_RangingMeasurementData_t;

class Adafruit_VL53L0X {
public:
    bool begin(uint8_t i2c_addr,
               bool debug,
               TwoWire *i2c) {
    }

    bool setAddress(uint8_t newAddr) { return false; }

    VL53L0X_Error
    rangingTest(VL53L0X_RangingMeasurementData_t *pRangingMeasurementData,
                bool debug = false) {
        // RangeStatus 4 is "out of range".
        pRangingMeasurementData->RangeStatus = 0;
        pRangingMeasurementData->RangeMilliMeter = 42;
    }
};
