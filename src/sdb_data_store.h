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

    void putLong(const SdbKey::SdbKey key, long value) {
        long* ptr = ptrLong(key, value);
        *ptr = value;
    }

    long getLong(const SdbKey::SdbKey key, long _default) {
        return *ptrLong(key, _default);
    }

    long* ptrLong(const SdbKey::SdbKey key, long _default) {
        auto key_value = _mapLong.find(key);
        if (key_value == _mapLong.end()) {
            long* val = (long*) calloc(1, sizeof(long));
            *val = _default;
            _mapLong[key] = val;
            return val;
        } else {
            return key_value->second;
        }
    }

    void putString(const SdbKey::SdbKey key, String& value) {
        String* ptr = ptrString(key, value);
        *ptr = value;
    }

    String& getString(const SdbKey::SdbKey key, String& _default) {
        return *ptrString(key, _default);
    }

    String* ptrString(const SdbKey::SdbKey key, String& _default) {
        auto key_value = _mapString.find(key);
        if (key_value == _mapString.end()) {
            String* val = (String*) calloc(1, sizeof(String));
            *val = _default;
            _mapString[key] = val;
            return val;
        } else {
            return key_value->second;
        }
    }
 

private:
    // An unordered map of SdbKey(as int) to long values.
    std::unordered_map<SdbKey::SdbKey, long*, std::hash<int>> _mapLong;
    // An unordered map of SdbKey(as int) to String values.
    std::unordered_map<SdbKey::SdbKey, String*, std::hash<int>> _mapString;
};

//
#endif // __INC_SDB_DATA_STORE_H

