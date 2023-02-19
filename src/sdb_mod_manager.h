#ifndef __INC_SDB_MOD_MANAGER_H
#define __INC_SDB_MOD_MANAGER_H

#include "common.h"
#include <vector>
#include "sdb_data_store.h"
#include "sdb_mod.h"

class SdbModManager {
public:
    SdbModManager() {
    }

    void registerMod(SdbMod* mod) {
        _mods.push_back(mod);
    }

    void onStart() {
        for(auto mod_p : _mods) {
            mod_p->onStart();
        }
    }

    void onLoop() {
        int pause_ms = 1000;
        for (auto mod_p : _mods) {
            int ms = mod_p->onLoop();
            if (ms > 0 && ms < pause_ms) {
                pause_ms = ms;
            }
        }
        DEBUG_PRINTF( ("loop pause %dms\n", pause_ms) );
        delay(pause_ms);
    }

    SdbDataStore& dataStore() {
        return _data_store;
    }


private:
    SdbDataStore _data_store;
    std::vector<SdbMod*> _mods;

};

//
#endif // __INC_SDB_MOD_MANAGER_H

