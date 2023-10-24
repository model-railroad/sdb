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
#include "sdb_data_store.h"

#include <Arduino_JSON.h>


//---------------

class SdbBlock {
public:
    explicit SdbBlock(SdbModManager& manager,
                   String&& name,
                   SdbSensor* sensor,
                   SdbKey::SdbKey keyNegate,
                   SdbKey::SdbKey keyJmriName,
                   SdbKey::SdbKey keyMqttTopic) :
        _manager(manager),
        _blockName(name),
        _sensor(sensor),
        _negate(false),
        _state(false),
        _keyNegate(keyNegate),
        _keyJmriName(keyJmriName),
        _keyMqttTopic(keyMqttTopic)
    {
        if (sensor == nullptr) {
            PANIC_PRINTF( ( "SdbBlock: Invalid sensor for block '%s'\n", name.c_str()) );
        }
    }

    const String& name() const {
        return _blockName;
    }

    void onStart() {
        _negate = _manager.dataStore().getLong(_keyNegate, 0) != 0;

        auto* jmriName = _manager.dataStore().getString(_keyJmriName);
        if (jmriName != nullptr) { _jmriName = *jmriName; }

        auto* mqttTopic = _manager.dataStore().getString(_keyMqttTopic);
        if (mqttTopic != nullptr) { _mqttTopic = *mqttTopic; }
    }

    /// Read current properties and fill in JSON var.
    JSONVar& getProperties(JSONVar &output) {
        JSONVar temp;

        output["bl:name.s"   ] = mkProp(temp, "Name",               name());
        output["bl:desc.s"   ] = mkProp(temp, "Description",        "Block Logic");
        output["bl.negate.b" ] = mkProp(temp, "Negate Output",      _negate ? "1" : "0");
        output["bl:sensor.s" ] = mkProp(temp, "Sensor Name",        _sensor->name());
        output["bl.jmname.s" ] = mkProp(temp, "JMRI System Name",   _jmriName);
        output["bl.mqtopic.s"] = mkProp(temp, "MQTT Topic",         _mqttTopic);
        output["bl!state.b"  ] = mkProp(temp, "State",              _state ? "1" : "0");

        return output;
    }

    /// Parse JSON var and store new mutable properties. Ignore non-mutable properties.
    void setProperties(JSONVar &input) {
        String negate = input["bl.negate.b"];       // empty if not set
        String jmName = input["bl.jmname.s"];       // empty if not set
        String mqTopic = input["bl.mqtopic.s"];     // empty if not set

        negate.trim();
        jmName.trim();
        mqTopic.trim();

        if (!negate.isEmpty()) {
            _negate = negate.toInt() != 0;
            _manager.dataStore().putLong(_keyNegate, _negate ? 1 : 0);
        }

        _jmriName = jmName;
        _manager.dataStore().putString(_keyJmriName, _jmriName);

        _mqttTopic = mqTopic;
        _manager.dataStore().putString(_keyMqttTopic, _mqttTopic);
    }

    /// Update and indicates if state has changed.
    bool update() {
        bool oldState = _state;
        _state = _sensor->state();
        return oldState != _state;
    }

private:
    SdbModManager& _manager;
    const String _blockName;
    const SdbSensor* _sensor;
    SdbKey::SdbKey _keyNegate;
    SdbKey::SdbKey _keyJmriName;
    SdbKey::SdbKey _keyMqttTopic;
    String _jmriName;
    String _mqttTopic;
    bool _negate;
    bool _state;
};

#endif // INC_SDB_BLOCK_H
