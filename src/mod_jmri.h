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

    void send(bool state, const String& jmriSystemName) {
        DEBUG_PRINTF( ("@@ JMRI host %s, port %d\n", _host.c_str(), _port) );

        // curl -d '{ "state": 4 }' -H "Content-Type: application/json" -X POST http://192.168.1.31:12080/json/sensor/NS785

        String payload("{ \"state\": ");
        payload += state ? JMRI_ACTIVE : JMRI_INACTIVE;
        payload += "}";

        String header("Content-Type: application/json");

        String path("/json/sensor/");
        path += jmriSystemName;

        HttpClient client = HttpClient(wifi, _host, _port);
        client.post(path, header, payload);

        int statusCode = client.responseStatusCode();
        String response = client.responseBody();
        DEBUG_PRINTF( ("@@ JMRI response code: %d -- %s\n", statusCode, response.c_str()) );
    }

private:
    WiFiClient wifi;
};

// --------------------------------

class SdbModJmri : public SdbMod {
public:
    explicit SdbModJmri(SdbModManager& manager) :
        SdbMod(manager, MOD_JMRI_NAME),
        _server(manager)
    { }

    void onStart() override {
        _manager.registerServer(&_server);
        _server.onStart();
    }

    long onLoop() override {
        for(;;) {
            auto event = dequeueEvent();
            switch(event.type) {
                case SdbEvent::Empty:
                    return 1000;
                case SdbEvent::BlockChanged:
                    _server.send(event.state, event.payload);
                    break;
                default:
                    // drop
                    break;
            }
        }

        return 1000;
    }

private:
    SdbServerJmri _server;

};


#endif // INC_SDB_MOD_JMRI_H
