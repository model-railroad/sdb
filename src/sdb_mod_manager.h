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
        _io_lock("LockIO")
    { }

    SdbLock& ioLock() {
        return _io_lock;
    }

    SdbDataStore& dataStore() {
        return _data_store;
    }

    void registerMod(SdbMod* mod) {
        _mods.push_back(mod);
    }

    long schedule(long delay_ms, const std::function<void()> lambda) {
        long now_ms = millis();
        Scheduled* scheduled = new Scheduled(now_ms + delay_ms, lambda);
        _scheduled.push_back(scheduled);
        // Vector sorted in reverse by _at_ms (sooner element at the end).
        std::sort(
            _scheduled.begin(),
            _scheduled.end(),
            [](Scheduled* a, Scheduled* b) { return a->_at_ms < b->_at_ms; }
        );
        if (_scheduled.empty()) {
            // This case cannot happen.
            return delay_ms;
        } else {
            // The latest element is the soonest, and indicates how much to wait.
            Scheduled* last = _scheduled.back();
            return last->_at_ms - now_ms;
        }
    }

    void onStart() {
        for(auto mod_p : _mods) {
            DEBUG_PRINTF( ("Start module [%s]\n", mod_p->name().c_str()) );
            mod_p->onStart();
        }
    }

    void onLoop() {
        long start_ms = millis();
        long next_ms = start_ms + 2000; // default: 2s loop

        while (!_scheduled.empty()) {
            Scheduled* last = _scheduled.back();
            if (last->_at_ms <= start_ms) {
                _scheduled.pop_back();
                last->_lambda();
            } else {
                if (last->_at_ms < next_ms) {
                    next_ms = last->_at_ms;
                }
                break;
            }
        }
    
        for (auto mod_p : _mods) {
            long mod_ms = millis();
            long ms = mod_p->onLoop();
            if (ms > 0) {
                mod_ms += ms;
            }
            if (mod_ms < next_ms) {
                next_ms = mod_ms;
            }
        }
        long loop_ms = millis() - start_ms;
        long delta_ms = next_ms - start_ms;
        DEBUG_PRINTF( ("loop %3d ms + pause %3d ms, sched #%d\n", loop_ms, delta_ms, _scheduled.size()) );
        if (delta_ms > 0) {
            delay(delta_ms);
        }
    }



private:
    SdbLock _io_lock;
    SdbDataStore _data_store;
    std::vector<SdbMod*> _mods;

    struct Scheduled {
        const long _at_ms;
        const std::function<void()> _lambda;
        Scheduled(const long at_ms, const std::function<void()> lambda):
            _at_ms(at_ms), _lambda(lambda) {
        }
    };
    // Vector sorted in reverse by _at_ms (sooner element at the end).
    std::vector<Scheduled*> _scheduled;
};

//
#endif // __INC_SDB_MOD_MANAGER_H

