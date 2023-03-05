#ifndef __INC_SDB_MOD_MANAGER_H
#define __INC_SDB_MOD_MANAGER_H

class SdbMod;

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

    long schedule(long delay_ms, const std::function<void()> lambda);

    void onStart();

    void onLoop();

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

