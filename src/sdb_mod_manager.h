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
        _ioLock("LockIO")
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

        while (!_scheduled.empty()) {
            Scheduled* last = _scheduled.back();
            if (last->_atMS <= startMS) {
                _scheduled.pop_back();
                last->_lambda();
            } else {
                if (last->_atMS < nextMS) {
                    nextMS = last->_atMS;
                }
                break;
            }
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
};

//
#endif // __INC_SDB_MOD_MANAGER_H

