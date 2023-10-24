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

#define MOD_JMRI_NAME "jm"

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

    // TBD customize stuff
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
        return 2000;
    }

private:
    SdbServerJmri _server;

};


#endif // INC_SDB_MOD_JMRI_H
