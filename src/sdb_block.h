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

#include <Arduino_JSON.h>

#include "common.h"
#include "mod_jmri.h"
#include "mod_mqtt.h"
#include "sdb_data_store.h"
#include "sdb_mod_manager.h"
#include "sdb_props.h"
#include "sdb_sensor.h"
#include "sdb_server.h"

//---------------

class SdbBlock {
public:
    explicit SdbBlock(SdbModManager& manager,
                   String&& name,
                   SdbSensor* sensor,
                   SdbKey::SdbKey keyInvert,
                   SdbKey::SdbKey keyRefresh,
                   SdbKey::SdbKey keyJmriName,
                   SdbKey::SdbKey keyMqttTopic) :
        _manager(manager),
        _blockName(name),
        _sensor(sensor),
        _invert(false),
        _state(false),
        _keyInvertLong(keyInvert),
        _keyRefreshLong(keyRefresh),
        _keyJmriNameStr(keyJmriName),
        _keyMqttTopicStr(keyMqttTopic),
        _lastNotifyTS(0),
        _refreshMS(5000)
    {
        if (sensor == nullptr) {
            PANIC_PRINTF( ( "SdbBlock: Invalid sensor for block '%s'\n", name.c_str()) );
        }
    }

    const String& name() const {
        return _blockName;
    }

    void onStart() {
        _invert = _manager.dataStore().getLong(_keyInvertLong, 0) != 0;
        _refreshMS = _manager.dataStore().getLong(_keyRefreshLong, _refreshMS);

        auto* jmriName = _manager.dataStore().getString(_keyJmriNameStr);
        if (jmriName != nullptr) { _jmriName = *jmriName; }

        auto* mqttTopic = _manager.dataStore().getString(_keyMqttTopicStr);
        if (mqttTopic != nullptr) { _mqttTopic = *mqttTopic; }
    }

    /// Read current properties and fill in JSON var.
    JSONVar& getProperties(JSONVar &output) {
        JSONVar temp;

        output["bl:name.s"   ] = mkProp(temp, "Name",               name());
        output["bl:desc.s"   ] = mkProp(temp, "Description",        "Block Logic");
        output["bl:sensor.s" ] = mkProp(temp, "Sensor Name",        _sensor->name());
        output["bl.refresh.i"] = mkProp(temp, "Refresh (ms)",       String(_refreshMS));
        output["bl.invert.b" ] = mkProp(temp, "Invert Output",      _invert ? "1" : "0");
        output["bl.jmname.s" ] = mkProp(temp, "JMRI System Name",   _jmriName);
        output["bl.mqtopic.s"] = mkProp(temp, "MQTT Topic",         _mqttTopic);
        output["bl!state.b"  ] = mkProp(temp, "State",              _state ? "1" : "0");

        return output;
    }

    /// Parse JSON var and store new mutable properties. Ignore non-mutable properties.
    void setProperties(JSONVar &input) {
        String invert = input["bl.invert.b"];       // empty if not set
        String jmName = input["bl.jmname.s"];       // empty if not set
        String mqTopic = input["bl.mqtopic.s"];     // empty if not set
        String refresh = input["bl.refresh.i"];     // empty if not set

        invert.trim();
        jmName.trim();
        mqTopic.trim();
        refresh.trim();

        if (!invert.isEmpty()) {
            _invert = invert.toInt() != 0;
            _manager.dataStore().putLong(_keyInvertLong, _invert ? 1 : 0);
        }

        if (!refresh.isEmpty()) {
            _refreshMS = refresh.toInt();
            _manager.dataStore().putLong(_keyRefreshLong, _refreshMS);
        }

        _jmriName = jmName;
        _manager.dataStore().putString(_keyJmriNameStr, _jmriName);

        _mqttTopic = mqTopic;
        _manager.dataStore().putString(_keyMqttTopicStr, _mqttTopic);
    }

    /// Update and indicates if state has changed.
    bool update() {
        bool oldState = _state;
        _state = _sensor->state();
        return oldState != _state;
    }

    void notify() {
        _lastNotifyTS = millis();
        DEBUG_PRINTF( ("@@ block %p notify [% 8d] %s -- state %d\n",
                      this, _lastNotifyTS, _jmriName.c_str(), _state) );
        if (!_jmriName.isEmpty()) {
            _manager.queueEvent(
                MOD_JMRI_NAME,
                SdbEvent::SdbEvent(SdbEvent::BlockChanged,
                                   _state,
                                   &_jmriName));
        }
        if (!_mqttTopic.isEmpty()) {
            _manager.queueEvent(
                MOD_MQTT_NAME,
                SdbEvent::SdbEvent(SdbEvent::BlockChanged,
                                   _state,
                                   &_mqttTopic));
        }
    }

    bool needsRefresh() const {
        if (_lastNotifyTS == 0) return true;
        if (_refreshMS <= 0) return false;
        return (millis() - _lastNotifyTS) > _refreshMS;
    }

private:
    SdbModManager& _manager;
    const String _blockName;
    const SdbSensor* _sensor;
    SdbKey::SdbKey _keyInvertLong;
    SdbKey::SdbKey _keyRefreshLong;
    SdbKey::SdbKey _keyJmriNameStr;
    SdbKey::SdbKey _keyMqttTopicStr;
    String _jmriName;
    String _mqttTopic;
    bool _invert;
    bool _state;
    unsigned long _lastNotifyTS;
    unsigned long _refreshMS;
};

#endif // INC_SDB_BLOCK_H
