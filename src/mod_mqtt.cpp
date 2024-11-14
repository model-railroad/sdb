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

module;

#include "common.h"
#include "mod_mqtt.h"

#include <functional>
#include <memory>
#include <WiFiClient.h>
#include <ArduinoMqttClient.h>

//---------------
export module SDB.Mod.MQTT;

import SDB.DataStore;
import SDB.Event;
import SDB.Mod;
import SDB.ModManager;
import SDB.PassDec;
import SDB.Server;

#define MQTT_ACTIVE "ACTIVE"
#define MQTT_INACTIVE "INACTIVE"

// --------------------------------

class SdbServerMqtt : public SdbServer {
public:
    explicit SdbServerMqtt(SdbModManager& manager) :
       SdbServer(manager,
                 "mqtt",
                 "MQTT Server",
                 SdbKey::ServerMqttHostStr,
                 SdbKey::ServerMqttPortLong)
    { }

    void onStart() override {
        SdbServer::onStart();

        auto* user = _manager.dataStore().getString(SdbKey::ServerMqttUserStr);
        if (user != nullptr) {
            _user = *user;
        }

        auto* pass = _manager.dataStore().getString(SdbKey::ServerMqttPassStr);
        if (pass != nullptr) {
            _pass = *pass;
        }

        auto* channel = _manager.dataStore().getString(SdbKey::ServerMqttChannelStr);
        if (channel != nullptr) {
            _channel = *channel;
        }
    }

    /// Read MQTT-specific current properties and fill in JSON var.
    JSONVar& getProperties(JSONVar &output) override {
        SdbServer::getProperties(output);

        JSONVar temp;
        // We don't send the actual password, just its length.
        String pass(_pass.length());
        output["mq.user.s"   ] = mkProp(temp, "MQTT User",      _user);
        output["mq*pass.s"   ] = mkProp(temp, "MQTT Password",  pass);
        output["mq.channel.s"] = mkProp(temp, "MQTT Channel",   _channel);
        return output;
    }

    /// Parse JSON var and store new mutable MQTT-specific properties. Ignore non-mutable properties.
    void setProperties(JSONVar &input) override {
        SdbServer::setProperties(input);

        SdbMutex lock(_propsLock);

        String user = input["mq.user.s"];        // empty if missing
        String channel = input["mq.channel.s"];  // empty if missing

        user.trim();
        bool changed = (_user != user);
        changed |= (_channel != channel);
        _user = user;
        _channel = channel;
        _manager.dataStore().putString(SdbKey::ServerMqttUserStr, user);
        _manager.dataStore().putString(SdbKey::ServerMqttChannelStr, channel);

        // Pass is not set if not changed.
        if (input.hasOwnProperty("mq*pass.s")) {
            String pass = sdbPassDec(input["mq*pass.s"]);
            changed |= (_pass == pass);
            _pass = pass;
            _manager.dataStore().putString(SdbKey::ServerMqttPassStr, pass);
        }

        _clientPropsChanged |= changed;
        DEBUG_PRINTF( ("[MQTT] Set Props. _clientPropsChanged %d\n", _clientPropsChanged) );
    }

    bool send(const String& mqttTopic, bool state) {
        if (mqttTopic.isEmpty()) return true;

        String topic(_channel);
        if (!topic.isEmpty()
            && topic.charAt(topic.length() - 1) != '/'
            && mqttTopic.charAt(0) != '/') {
            topic += "/";
        }
        topic += mqttTopic;

        String payload(state ? MQTT_ACTIVE : MQTT_INACTIVE);

        millis_t postTS = millis();  // for debug purposes below

        // Note: MqttClient methods returns 0 on error, 1 on success.
        int result = _client->beginMessage(topic);
        if (result) _client->print(payload);
        result = result && _client->endMessage();

        DEBUG_PRINTF( ("[MQTT] [%s = %s] -- publish time: %d ms, result %d\n",
                      topic.c_str(),
                      payload.c_str(),
                      millis() - postTS,
                      result) );
        return result == 1;
    }

    void connect() {
        if (!isConnected() || _clientPropsChanged) {
            SdbMutex lock(_propsLock);
            _clientPropsChanged = false;
            if (_client) {
                _client->stop();
            }
            DEBUG_PRINTF(("[MQTT] Connect to host %s, port %d\n", _host.c_str(), _port));
            _client.reset(new MqttClient(_wifi));
            _client->setUsernamePassword(_user, _pass);
            // Note: MqttClient methods returns 0 on error, 1 on success.
            // For errors, the error code is returned via connectError().
            int result = _client->connect(_host.c_str(), _port);
            DEBUG_PRINTF(("[MQTT] Connect result %d, error %d\n", result, _client->connectError()));
            if (result == 0 && _client->connectError() < MQTT_SUCCESS) {
                // A negative result (e.g. conx refused) can happen if the wifi had not finished
                // connecting yet. In that case, delete the client so that we can try again later.
                _client.reset(nullptr);
            }
        }
    }

    bool isConnected() {
        return _client.operator bool() && _client->connected();
    }

    void clientLoop() {
        if (_client) {
            _client->poll();
        }
    }

private:
    WiFiClient _wifi;
    /// An MqttClient client. Reminder: this API seems to return 1 on success, 0 on error.
    std::unique_ptr<MqttClient> _client;

    String _user;
    String _pass;
    String _channel;
};

// --------------------------------

class SdbModMqtt : public SdbModTask {
public:
    explicit SdbModMqtt(SdbModManager& manager) :
       SdbModTask(manager, MOD_MQTT_NAME, "TaskMqtt", SdbPriority::Network),
        _server(manager)
    { }

    void onStart() override {
        _manager.registerServer(_server);
        _server.onStart();
        startTask();
    }

    millis_t onLoop() override {
        return 2000;
    }

private:
    SdbServerMqtt _server;
    std::map<String, std::unique_ptr<SdbEvent::SdbEvent>> _events;

    [[noreturn]] void onTaskRun() override {
        while (true) {
            _server.clientLoop();

            if (hasEvents()) {
                do {
                    auto event = dequeueEvent();
                    if (event && event->type == SdbEvent::BlockChanged) {
                        auto eventBlock = reinterpret_cast<SdbEvent::SdbEventBlockChanged *>(event.get());
                        const String& key = eventBlock->payload;
                        _events[key] = std::move(event);
                    }
                } while (hasEvents());

                if (!_events.empty()) {
                    _server.connect();
                }

                if (!_server.isConnected()) {
                    DEBUG_PRINTF(("[MQTT] %d pending events, NOT CONNECTED.\n", _events.size()));
                } else {
                    bool success = true;
                    for (const auto& [key, event] : _events) {
                        auto eventBlock = reinterpret_cast<SdbEvent::SdbEventBlockChanged *>(event.get());
                        if (_server.send(key, eventBlock->state)) {
                            _events.erase(key);
                        } else {
                            success = false;
                        }
                    }
                    if (success) {
                        _events.clear();
                    }
                    BLINK_EVENT(_manager, success ? SdbBlinkMode::STAPublishOk : SdbBlinkMode::STAPublishFail);
                }
            }

            rtDelay(250L);
        }
    }
};
