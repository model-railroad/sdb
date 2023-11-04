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

#ifndef INC_SDB_SENSOR_H
#define INC_SDB_SENSOR_H

class SdbModManager;

#include "common.h"
#include "sdb_mod_manager.h"

#include <Arduino_JSON.h>
#include <U8g2lib.h>
#include <vector>


//---------------

class ISdbSensorBlockLogic;

class SdbSensor {
public:
    SdbSensor(SdbModManager& manager, String&& name) :
        _manager(manager),
        _sensorName(name)
    { }

    const String& name() const {
        return _sensorName;
    }

    virtual ISdbSensorBlockLogic* makeLogic(int blockIndex) = 0;

    /// Read current properties and fill in JSON var.
    virtual JSONVar& getProperties(JSONVar &output) = 0;

    /// Parse JSON var and store new mutable properties. Ignore non-mutable properties.
    virtual void setProperties(JSONVar &input) = 0;

    #if defined(USE_DISPLAY_LIB_U8G2)
    // Warning: this executes in the display task.
    virtual void draw(U8G2_SSD1306_128X64_NONAME_F_HW_I2C& _u8g2, int yOffset) = 0;
    #endif

protected:
    SdbModManager& _manager;

private:
    const String _sensorName;
};

class ISdbSensorBlockLogic {
public:
    virtual ~ISdbSensorBlockLogic() = default;

    virtual void init() = 0;

    /// Computes the current state of the sensor viewed from this block's properties.
    virtual bool state() const = 0;

    /// Read current block-specific properties for this sensor and fill in JSON var.
    virtual JSONVar& getProperties(JSONVar &output) = 0;

    /// Parse JSON var and store new mutable properties. Ignore non-mutable properties.
    virtual void setProperties(JSONVar &input) = 0;
};

template <class S>
class SdbSensorBlockLogic : public ISdbSensorBlockLogic {
public:
 SdbSensorBlockLogic(SdbModManager& manager, S* sensor) :
       _manager(manager),
       _sensor(sensor)
    { }

protected:
    SdbModManager& _manager;
    S* _sensor;
};

#endif // INC_SDB_SENSOR_H
