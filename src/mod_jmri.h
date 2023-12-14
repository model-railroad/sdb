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

#ifndef INC_SDB_MOD_JMRI_H
#define INC_SDB_MOD_JMRI_H

#include "common.h"
#include "sdb_mod.h"
#include "sdb_server.h"

#include <memory>
#include <WiFiClient.h>
#include <ArduinoHttpClient.h>

#define MOD_JMRI_NAME "jm"

#define JMRI_ACTIVE "2"
#define JMRI_INACTIVE "4"


// --------------------------------

class SdbServerJmri : public SdbServer {
public:
    explicit SdbServerJmri(SdbModManager& manager) :
       SdbServer(manager,
                 "jmri",
                 "JMRI Server",
                 SdbKey::ServerJmriHostStr,
                 SdbKey::ServerJmriPortLong)
    { }

    void send(const String& jmriSystemName, bool state) {
        DEBUG_PRINTF( ("@@ JMRI host %s, port %d\n", _host.c_str(), _port) );

        // curl -d '{ "state": 4 }' -H "Content-Type: application/json" -X POST http://192.168.1.31:12080/json/sensor/NS785

        String payload("{ \"state\": ");
        payload += state ? JMRI_ACTIVE : JMRI_INACTIVE;
        payload += "}";

        String header("Content-Type: application/json");

        String path("/json/sensor/");
        path += jmriSystemName;

        if (!_client) {
            _client.reset(new HttpClient(_wifi, _host, _port));
            _client->connectionKeepAlive();
        }
        millis_t postTS = millis(); // for debug purposes below
        _client->post(path, header, payload);

        int statusCode = _client->responseStatusCode();
        String response = _client->responseBody();
        DEBUG_PRINTF( ("@@ JMRI [%s = %s] -- response delay: %d ms, code: %d -- %s\n",
                      jmriSystemName.c_str(),
                      (state ? "ON" : "OFF"),
                      millis() - postTS,
                      statusCode,
                      response.c_str()) );

    }

private:
    WiFiClient _wifi;
    std::unique_ptr<HttpClient> _client;
};

// --------------------------------

class SdbModJmri : public SdbModTask {
public:
    explicit SdbModJmri(SdbModManager& manager) :
       SdbModTask(manager, MOD_JMRI_NAME, "TaskJmri", SdbPriority::Network),
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
    SdbServerJmri _server;
    std::map<String, SdbEvent::SdbEvent> _events;

    [[noreturn]] void onTaskRun() override {
        while(true) {

            if (hasEvents()) {
                do {
                    auto event = dequeueEvent();
                    if (event.type == SdbEvent::BlockChanged) {
                        const String* key = event.data;
                        _events[*key] = event;
                    }
                } while (hasEvents());

                for (const auto& [key, event]: _events) {
                    // Each send blocks (measured to be around ~1050 ms).
                    _server.send(key, event.state);
                }
                _events.clear();
            }

            rtDelay(250L);
        }
    }
};


#endif // INC_SDB_MOD_JMRI_H
