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

#ifndef INC_SDB_SERVER_H
#define INC_SDB_SERVER_H

#include "common.h"
#include "sdb_mod_manager.h"
#include "sdb_props.h"

#include <Arduino_JSON.h>


//---------------

class SdbServer {
public:
    explicit SdbServer(SdbModManager& manager,
                    String&& name,
                    String&& description,
                    SdbKey::SdbKey keyHost,
                    SdbKey::SdbKey keyPort) :
        _manager(manager),
        _serverName(name),
        _description(description),
        _keyHost(keyHost),
        _keyPort(keyPort)
    { }

    const String& name() const {
        return _serverName;
    }

    void onStart() {
        _port = _manager.dataStore().getLong(_keyPort, 0);

        auto* host = _manager.dataStore().getString(_keyHost);
        if (host != nullptr) { _host = *host; }
    }

    /// Read current properties and fill in JSON var.
    JSONVar& getProperties(JSONVar &output) {
        JSONVar temp;

        output["sv:name.s"  ] = mkProp(temp, "Name",        _serverName);
        output["sv:desc.s"  ] = mkProp(temp, "Description", _description);
        output["sv.host.s"  ] = mkProp(temp, "Host IP",     _host);
        output["sv.port.i"  ] = mkProp(temp, "Port",        String(_port));

        return output;
    }

    /// Parse JSON var and store new mutable properties. Ignore non-mutable properties.
    void setProperties(JSONVar &input) {
        String host = input["sv.host.s"];       // empty if not set
        String port = input["sv.port.i"];     // empty if not set

        host.trim();
        port.trim();

        _host = host;
        _manager.dataStore().putString(_keyHost, _host);

        if (!port.isEmpty()) {
            _port = port.toInt();
            _manager.dataStore().putLong(_keyPort, _port);
        }
    }

private:
    SdbModManager& _manager;
    const String _serverName;
    const String _description;
    SdbKey::SdbKey _keyHost;
    SdbKey::SdbKey _keyPort;
    String _host;
    int _port;
};

#endif // INC_SDB_SERVER_H
