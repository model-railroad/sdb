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

#ifndef INC_SDB_MOD_MQTT_H
#define INC_SDB_MOD_MQTT_H

#include "common.h"
#include "sdb_mod.h"
#include "sdb_server.h"

#define MOD_MQTT_NAME "mq"

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

    void send(bool state, const String& mqttTopic) {
        DEBUG_PRINTF( ("@@ MQTT host %s, port %d\n", _host.c_str(), _port) );

        // TBD customize stuff
    }

};

// --------------------------------

class SdbModMqtt : public SdbModTask {
public:
    explicit SdbModMqtt(SdbModManager& manager) :
       SdbModTask(manager, MOD_MQTT_NAME, "TaskMqtt", SdbPriority::Network),
        _server(manager)
    { }

    void onStart() override {
        _manager.registerServer(&_server);
        _server.onStart();
        startTask();
    }

    millis_t onLoop() override {
        return 2000;
    }

private:
    SdbServerMqtt _server;
    std::map<String, SdbEvent::SdbEvent> _events;

    [[noreturn]] void onTaskRun() override {
       while(true) {

           if (hasEvents()) {
               _events.clear();
               do {
                   auto event = dequeueEvent();
                   if (event.type == SdbEvent::BlockChanged) {
                       _events[*event.data] = event;
                   }
               } while (hasEvents());

               for (const auto& [key, event]: _events) {
                   _server.send(event.state, key);
               }
               _events.clear();
           }

           rtDelay(250L);
       }
    }

};


#endif // INC_SDB_MOD_MQTT_H
