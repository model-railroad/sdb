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
#include <memory>
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
    /// This panics if the module name is not unique and has already been registered.
    void registerMod(const std::shared_ptr<SdbMod>& mod) {
        if (modByName(mod->name()) != nullptr) {
            PANIC_PRINTF( ("Mod Manager: Mod name '%s' cannot be redefined\n", mod->name().c_str()) );
        }
        _modsmap[mod->name()] = mod;
        _mods.push_back(mod);
    }

    /// Returns a pointer on a module, or null if the module name does not exist.
    SdbMod* modByName(const String& modName) const {
        // Optimize this using a map, as it's used by queueEvent().
        auto kvNameMod = _modsmap.find(modName);
        if (kvNameMod == _modsmap.end()) {
            return nullptr;
        } else {
            return kvNameMod->second.get();
        }
    }

    /// Registers a new sensor.
    /// Synchronization: None. This MUST be called at init or start time.
    void registerSensor(SdbSensor& sensor) {
        _sensors.emplace_back(sensor);
    }

    const std::vector<std::reference_wrapper<SdbSensor>>& sensors() const {
        return _sensors;
    }

    /// Returns a pointer on a sensor, or null if the sensor does not exist.
    SdbSensor* sensorByName(const String& sensorName) const {
        for (auto& sensor: _sensors) {
            if (sensorName == sensor.get().name()) {
                return &(sensor.get());
            }
        }
        return nullptr;
    }

    /// Registers a new server.
    /// Synchronization: None. This MUST be called at init or start time.
    void registerServer(SdbServer& server) {
        _servers.emplace_back(server);
    }

    const std::vector<std::reference_wrapper<SdbServer>>& servers() const {
        return _servers;
    }

    /// Registers a new block.
    /// Synchronization: None. This MUST be called at init or start time.
    /// TBD: later add synchronization so that it can be dynamic.
    void registerBlock(const std::shared_ptr<SdbBlock>& block) {
        _blocks.push_back(block);
    }

    const std::vector<std::shared_ptr<SdbBlock>>& blocks() const {
        return _blocks;
    }

    void queueEvent(const String& modName, SdbEvent::Type eventType) const {
        auto event = std::unique_ptr<SdbEvent::SdbEvent>(new SdbEvent::SdbEvent(eventType));
        queueEvent(modName, std::move(event));
    }

    void queueEvent(const String& modName, std::unique_ptr<SdbEvent::SdbEvent> event) const {
        SdbMod* mod = modByName(modName);
        if (mod == nullptr) {
            PANIC_PRINTF( ("QueueEvent: Unknown mod name '%s'\n", modName.c_str()) );
        }
        mod->queueEvent(std::move(event));
    }

    millis_t schedule(millis_t delayMS, const std::function<void()>& lambda) {
        SdbMutex autoMutex(_scheduleLock);
        millis_t nowMS = millis();
        auto scheduled = std::unique_ptr<Scheduled>(new Scheduled(nowMS + delayMS, lambda));
        _scheduled.push_back(std::move(scheduled));
        // Vector sorted in reverse by _atMS (sooner element at the end).
        std::sort(
            _scheduled.begin(),
            _scheduled.end(),
            [](const std::unique_ptr<Scheduled>& a, const std::unique_ptr<Scheduled>& b) {
                return a->_atMS > b->_atMS;
            }
        );
        if (_scheduled.empty()) {
            // This case cannot happen.
            return delayMS;
        } else {
            // The latest element is the soonest, and indicates how much to wait.
            auto& last = _scheduled.back();
            return last->_atMS - nowMS;
        }
    }

    void onStart() {
        _dataStore.onStart();
        for(auto& mod: _mods) {
            DEBUG_PRINTF( ("Start module [%s]\n", mod->name().c_str()) );
            mod->onStart();
        }
    }

    void onLoop() {
        // When we start this loop iteration.
        millis_t startMS = millis();
        // The earliest time we want the *next* loop iteration to be. Default is in 2 seconds,
        // and we'll adjust that if there's some action to perform sooner than that.
        millis_t nextMS = startMS + 2000;

        while (true) {
            std::unique_ptr<Scheduled> last;
            {
                SdbMutex autoMutex(_scheduleLock);
                if (_scheduled.empty()) {
                    break;
                }
                // Scheduled vector is sorted with the earliest action to execute at the
                // end of the vector.
                auto& peak = _scheduled.back();
                if (peak->_atMS <= startMS) {
                    // Action was due before start of loop. Must execute it now.
                    // Exec the lambda below out of the lock
                    last = std::move(peak);
                    _scheduled.pop_back();
                } else {
                    // The first action to execute is in the future. Adjust the next loop iteration
                    // to match.
                    if (peak->_atMS < nextMS) {
                        nextMS = peak->_atMS;
                    }
                    break;
                }
            }
            if (last) {
                last->_lambda();
            }
        }
    
        for (auto& mod : _mods) {
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
        auto size = _scheduled.size();
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
    std::vector<std::shared_ptr<SdbMod>> _mods;
    std::map<String, std::shared_ptr<SdbMod>> _modsmap;
    std::vector<std::reference_wrapper<SdbSensor>> _sensors;
    std::vector<std::reference_wrapper<SdbServer>> _servers;
    std::vector<std::shared_ptr<SdbBlock>> _blocks;
    long _debug_printf;

    struct Scheduled {
        const millis_t _atMS;
        const std::function<void()> _lambda;
        Scheduled(const millis_t at_ms, const std::function<void()>& lambda):
            _atMS(at_ms), _lambda(lambda) {
        }
    };
    // Vector sorted in reverse by _atMS (sooner element at the end).
    std::vector<std::unique_ptr<Scheduled>> _scheduled;
    SdbLock _scheduleLock;
};

//
#endif // INC_SDB_MOD_MANAGER_H

