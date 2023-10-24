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

#ifndef INC_SDB_BLOCK_H
#define INC_SDB_BLOCK_H

#include "common.h"
#include "sdb_mod_manager.h"
#include "sdb_props.h"
#include "sdb_sensor.h"

#include <Arduino_JSON.h>


//---------------

class SdbBlock {
public:
    explicit SdbBlock(String&& name, SdbSensor* sensor) :
        _blockName(name),
        _sensor(sensor),
        _negate(false),
        _state(false)
    {
        if (sensor == nullptr) {
            PANIC_PRINTF( ( "SdbBlock: Invalid sensor for block '%s'\n", name.c_str()) );
        }
    }

    const String& name() const {
        return _blockName;
    }

    /// Read current properties and fill in JSON var.
    JSONVar& getProperties(JSONVar &output) {
        JSONVar temp;

        output["bl.name.s"   ] = mkProp(temp, "Name",               name());
        output["bl.desc.s"   ] = mkProp(temp, "Description",        "Block Logic");
        output["bl.negate.b" ] = mkProp(temp, "Negate Output",      _negate ? "1" : "0");
        output["bl.sensor.s" ] = mkProp(temp, "Sensor Name",        _sensor->name());
        output["bl.jmname.s" ] = mkProp(temp, "JMRI System Name",   _jmriName);
        output["bl.mqtopic.s"] = mkProp(temp, "MQTT Topic",         _mqttTopic);
        output["bl!state.b"  ] = mkProp(temp, "State",              _state ? "1" : "0");

        return output;
    }

    /// Parse JSON var and store new mutable properties. Ignore non-mutable properties.
    void setProperties(JSONVar &input) {
        String negate = input["bl.negate.b"];       // empty if not set
        //String sensor = input["bl.sensor.s"];       // empty if not set -- cannot be changed
        String jmName = input["bl.jmname.s"];       // empty if not set
        String mqTopic = input["bl.mqtopic.s"];     // empty if not set

        negate.trim();
        //sensor.trim();
        jmName.trim();
        mqTopic.trim();

        // TBD validate
        _negate = negate == "1";
        //--_sensorName = sensor;
        _jmriName = jmName;
        _mqttTopic = mqTopic;

        // TBD init from NVS, save to NVS
        //  ...  _manager.dataStore().putLong(_maxKey, maxThreshold);
    }

    /// Update and indicates if state has changed.
    bool update() {
        bool oldState = _state;
        _state = _sensor->state();
        return oldState != _state;
    }

private:
    const String _blockName;
    const SdbSensor* _sensor;
    String _jmriName;
    String _mqttTopic;
    bool _negate;
    bool _state;
};

#endif // INC_SDB_BLOCK_H
