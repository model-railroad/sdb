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

#ifndef INC_SDB_MOD_MANAGER_H
#define INC_SDB_MOD_MANAGER_H

#include "common.h"
#include "sdb_data_store.h"
#include "sdb_lock.h"
#include "sdb_mod.h"
#include "sdb_sensor.h"
#include <algorithm>
#include <functional>
#include <map>
#include <vector>

class SdbBlock;     // avoid #include "sdb_block.h"  due to circular inclusions.
class SdbServer;    // avoid #include "sdb_server.h" due to circular inclusions.

class SdbModManager {
public:
    SdbModManager() :
        _ioLock("LockIO"),
        _scheduleLock("LockSched"),
        _debug_printf(0)
    { }

    SdbLock& ioLock() {
        return _ioLock;
    }

    SdbDataStore& dataStore() {
        return _dataStore;
    }

    /// Registers a new module.
    /// Modules are executed in the order they are defined.
    /// Synchronization: None. This MUST be called at init or start time.
    void registerMod(SdbMod* mod) {
        _mods.push_back(mod);
        _modsmap[mod->name()] = mod;
    }

    SdbMod* modByName(const String& modName) const {
        // Optimize this using a map, as its used by queueEvent().
        auto kvNameMod = _modsmap.find(modName);
        if (kvNameMod == _modsmap.end()) {
            return nullptr;
        } else {
            return kvNameMod->second;
        }
    }

    /// Registers a new sensor.
    /// Synchronization: None. This MUST be called at init or start time.
    void registerSensor(SdbSensor* sensor) {
        _sensors.push_back(sensor);
    }

    const std::vector<SdbSensor*>& sensors() const {
        return _sensors;
    }

    SdbSensor* sensorByName(const String& sensorName) const {
        for (auto* sensor: _sensors) {
            if (sensorName == sensor->name()) {
                return sensor;
            }
        }
        return null;
    }

    /// Registers a new server.
    /// Synchronization: None. This MUST be called at init or start time.
    void registerServer(SdbServer* server) {
        _servers.push_back(server);
    }

    const std::vector<SdbServer*>& servers() const {
        return _servers;
    }

    /// Registers a new block.
    /// Synchronization: None. This MUST be called at init or start time.
    /// TBD: later add synchronization so that it can be dynamic.
    void registerBlock(SdbBlock* block) {
        _blocks.push_back(block);
    }

    const std::vector<SdbBlock*>& blocks() const {
        return _blocks;
    }

    void queueEvent(const String& modName, const SdbEvent::SdbEvent& event) {
        SdbMod* mod = modByName(modName);
        if (mod == nullptr) {
            PANIC_PRINTF( ("QueueEvent: Unknown mod name '%s'\n", modName.c_str()) );
        }
        mod->queueEvent(event);
    }

    long schedule(millis_t delayMS, const std::function<void()> lambda) {
        SdbMutex autoMutex(_scheduleLock);
        millis_t nowMS = millis();
        auto* scheduled = new Scheduled(nowMS + delayMS, lambda);
        _scheduled.push_back(scheduled);
        // Vector sorted in reverse by _atMS (sooner element at the end).
        std::sort(
            _scheduled.begin(),
            _scheduled.end(),
            [](Scheduled* a, Scheduled* b) { return a->_atMS < b->_atMS; }
        );
        if (_scheduled.empty()) {
            // This case cannot happen.
            return delayMS;
        } else {
            // The latest element is the soonest, and indicates how much to wait.
            Scheduled* last = _scheduled.back();
            return last->_atMS - nowMS;
        }
    }

    void onStart() {
        _dataStore.onStart();
        for(auto* mod: _mods) {
            DEBUG_PRINTF( ("Start module [%s]\n", mod->name().c_str()) );
            mod->onStart();
        }
    }

    void onLoop() {
        millis_t startMS = millis();
        millis_t nextMS = startMS + 2000; // default: 2s loop

        while (true) {
            Scheduled* last = NULL;
            {
                SdbMutex autoMutex(_scheduleLock);
                if (_scheduled.empty()) {
                    break;
                }
                // the last item is the soonest we need to execute
                last = _scheduled.back();
                // last cannot be null below
                if (last->_atMS <= startMS) {
                    _scheduled.pop_back();
                    // exec the lambda below out of the lock
                } else {
                    if (last->_atMS < nextMS) {
                        nextMS = last->_atMS;
                    }
                    break;
                }
            }
            last->_lambda();
            delete last;
        }
    
        for (auto* mod : _mods) {
            millis_t modMS = millis();
            millis_t ms = mod->onLoop();
            if (ms > 0) {
                modMS += ms;
            }
            if (modMS < nextMS) {
                nextMS = modMS;
            }
        }
        millis_t loopMS = millis() - startMS;
        millis_t deltaMS = nextMS - startMS;
        int size = _scheduled.size();
        long debug_printf = loopMS * 1000 + deltaMS + size;
        if (debug_printf != _debug_printf) {
            DEBUG_PRINTF( ("loop %3d ms + pause %3d ms, sched #%d\n", loopMS, deltaMS, size) );
            _debug_printf = debug_printf;
        }
        if (deltaMS > 0) {
            delay(deltaMS);
        }
    }



private:
    SdbLock _ioLock;
    SdbDataStore _dataStore;
    std::vector<SdbMod*> _mods;
    std::map<String, SdbMod*> _modsmap;
    std::vector<SdbSensor*> _sensors;
    std::vector<SdbServer*> _servers;
    std::vector<SdbBlock*> _blocks;
    long _debug_printf;

    struct Scheduled {
        const millis_t _atMS;
        const std::function<void()> _lambda;
        Scheduled(const millis_t at_ms, const std::function<void()> lambda):
            _atMS(at_ms), _lambda(lambda) {
        }
    };
    // Vector sorted in reverse by _atMS (sooner element at the end).
    std::vector<Scheduled*> _scheduled;
    SdbLock _scheduleLock;
};

//
#endif // INC_SDB_MOD_MANAGER_H

