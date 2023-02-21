#ifndef __INC_SDB_DATA_STORE_H
#define __INC_SDB_DATA_STORE_H

#include "common.h"
#include "sdb_mod_manager.h"
#include <unordered_map>

namespace SdbKey {
    enum SdbKey {
        Empty,
        TofDistanceMM,
    };
}

class SdbDataStore {
public:
    SdbDataStore() {
    }

    void put(const SdbKey::SdbKey key, long value) {
        _map[key] = value;
    }

    long get(const SdbKey::SdbKey key, long _default) {
        auto key_value = _map.find(key);
        if (key_value == _map.end()) {
            return _default;
        } else {
            return key_value->second;
        }
    }
    

private:
    // An unordered map of SdbKey(as int) to long values.
    std::unordered_map<SdbKey::SdbKey, long, std::hash<int>> _map;
};

//
#endif // __INC_SDB_DATA_STORE_H

