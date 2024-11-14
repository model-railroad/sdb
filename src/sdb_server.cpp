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

#include <Arduino_JSON.h>

//---------------
export module SDB.Server;

import SDB.Lock;
import SDB.ModManager;
import SDB.Props;

export class SdbServer {
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
        _keyPort(keyPort),
       _clientPropsChanged(false),
       _propsLock(("propsLock"))
    { }

    const String& name() const {
        return _serverName;
    }

    virtual void onStart() {
        _port = _manager.dataStore().getLong(_keyPort, 0);

        auto* host = _manager.dataStore().getString(_keyHost);
        if (host != nullptr) { _host = *host; }
    }

    /// Read current properties and fill in JSON var.
    virtual JSONVar& getProperties(JSONVar &output) {
        JSONVar temp;

        output["sv:name.s"  ] = mkProp(temp, "Name",        _serverName);
        output["sv:desc.s"  ] = mkProp(temp, "Description", _description);
        output["sv.host.s"  ] = mkProp(temp, "Host IP",     _host);
        output["sv.port.i"  ] = mkProp(temp, "Port",        String(_port));

        return output;
    }

    /// Parse JSON var and store new mutable properties. Ignore non-mutable properties.
    virtual void setProperties(JSONVar &input) {
        SdbMutex lock(_propsLock);
        String host = input["sv.host.s"];       // empty if not set
        String port = input["sv.port.i"];       // empty if not set

        host.trim();
        port.trim();

        bool changed = (_host != host);
        _host = host;
        _manager.dataStore().putString(_keyHost, host);

        if (!port.isEmpty()) {
            long newPort = port.toInt();
            changed |= _port != newPort;
            _port = newPort;
            _manager.dataStore().putLong(_keyPort, newPort);
        }

        _clientPropsChanged |= changed;
    }

protected:
    SdbModManager& _manager;
    const String _serverName;
    const String _description;
    SdbKey::SdbKey _keyHost;
    SdbKey::SdbKey _keyPort;
    String _host;
    int _port;
    SdbLock _propsLock;
    bool _clientPropsChanged;
};
