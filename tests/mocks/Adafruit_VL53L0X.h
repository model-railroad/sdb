/*
 * Project: Software Defined Blocks
 * Copyright (C) 2024 alf.labs gmail com.
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
