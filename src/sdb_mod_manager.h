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

#ifndef __INC_SDB_MOD_MANAGER_H
#define __INC_SDB_MOD_MANAGER_H

#include "common.h"
#include "sdb_data_store.h"
#include "sdb_lock.h"
#include "sdb_mod.h"
#include <algorithm>
#include <functional>
#include <map>
#include <vector>

class SdbModManager {
public:
    SdbModManager() :
        _ioLock("LockIO"),
        _scheduleLock("LockSched")
    { }

    SdbLock& ioLock() {
        return _ioLock;
    }

    SdbDataStore& dataStore() {
        return _dataStore;
    }

    void registerMod(SdbMod* mod) {
        _mods[mod->name()] = mod;
    }

    SdbMod* modByName(const String& modName) {
        auto kvNameMod = _mods.find(modName);
        if (kvNameMod == _mods.end()) {
            return NULL;
        } else {
            return kvNameMod->second;
        }
    }

    void queueEvent(const String& modName, const SdbEvent::SdbEvent event) {
        SdbMod* mod = modByName(modName);
        assert(mod != NULL);
        mod->queueEvent(event);
    }

    long schedule(long delayMS, const std::function<void()> lambda) {
        SdbMutex autoMutex(_scheduleLock);
        long nowMS = millis();
        Scheduled* scheduled = new Scheduled(nowMS + delayMS, lambda);
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
        for(auto kvNameMod : _mods) {
            DEBUG_PRINTF( ("Start module [%s]\n", kvNameMod.first.c_str()) );
            kvNameMod.second->onStart();
        }
    }

    void onLoop() {
        long startMS = millis();
        long nextMS = startMS + 2000; // default: 2s loop

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
    
        for (auto kvNameMod : _mods) {
            long modMS = millis();
            long ms = kvNameMod.second->onLoop();
            if (ms > 0) {
                modMS += ms;
            }
            if (modMS < nextMS) {
                nextMS = modMS;
            }
        }
        long loopMS = millis() - startMS;
        long deltaMS = nextMS - startMS;
        DEBUG_PRINTF( ("loop %3d ms + pause %3d ms, sched #%d\n", loopMS, deltaMS, _scheduled.size()) );
        if (deltaMS > 0) {
            delay(deltaMS);
        }
    }



private:
    SdbLock _ioLock;
    SdbDataStore _dataStore;
    std::map<String, SdbMod*> _mods;

    struct Scheduled {
        const long _atMS;
        const std::function<void()> _lambda;
        Scheduled(const long at_ms, const std::function<void()> lambda):
            _atMS(at_ms), _lambda(lambda) {
        }
    };
    // Vector sorted in reverse by _atMS (sooner element at the end).
    std::vector<Scheduled*> _scheduled;
    SdbLock _scheduleLock;
};

//
#endif // __INC_SDB_MOD_MANAGER_H

