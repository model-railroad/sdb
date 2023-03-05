#ifndef __INC_SDB_MOD_MANAGER_H
#define __INC_SDB_MOD_MANAGER_H

#include "common.h"
#include <algorithm>
#include <functional>
#include <vector>
#include "sdb_data_store.h"
#include "sdb_lock.h"
#include "sdb_mod.h"

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
        _mods.push_back(mod);
    }

    long schedule(long delay_ms, const std::function<void()> lambda) {
        long now_ms = millis();
        Scheduled* scheduled = new Scheduled(now_ms + delay_ms, lambda);
        _scheduled.push_back(scheduled);
        // Vector sorted in reverse by _atMS (sooner element at the end).
        std::sort(
            _scheduled.begin(),
            _scheduled.end(),
            [](Scheduled* a, Scheduled* b) { return a->_atMS < b->_atMS; }
        );
        if (_scheduled.empty()) {
            // This case cannot happen.
            return delay_ms;
        } else {
            // The latest element is the soonest, and indicates how much to wait.
            Scheduled* last = _scheduled.back();
            return last->_atMS - now_ms;
        }
    }

    void onStart() {
        for(auto mod_p : _mods) {
            DEBUG_PRINTF( ("Start module [%s]\n", mod_p->name().c_str()) );
            mod_p->onStart();
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
    
        for (auto mod_p : _mods) {
            long modMS = millis();
            long ms = mod_p->onLoop();
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
    std::vector<SdbMod*> _mods;

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

