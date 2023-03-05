#ifndef __INC_SDB_DATA_STORE_H
#define __INC_SDB_DATA_STORE_H

#include "common.h"
#include "sdb_lock.h"
#include <unordered_map>

namespace SdbKey {
    enum SdbKey {
        Empty,
        TofDistanceMM,
    };
}

class SdbDataStore {
public:
    SdbDataStore() :
        _lock("LockData")
    { }

    SdbLock& lock() {
        return _lock;
    }

    void putLong(const SdbKey::SdbKey key, long value);

    long getLong(const SdbKey::SdbKey key, long _default);

    long* ptrLong(const SdbKey::SdbKey key, long _default);

    void putString(const SdbKey::SdbKey key, String& value);

    String& getString(const SdbKey::SdbKey key, String& _default);

    String* ptrString(const SdbKey::SdbKey key, String& _default);

private:
    SdbLock _lock;
    // An unordered map of SdbKey(as int) to long values.
    std::unordered_map<SdbKey::SdbKey, long*, std::hash<int>> _mapLong;
    // An unordered map of SdbKey(as int) to String values.
    std::unordered_map<SdbKey::SdbKey, String*, std::hash<int>> _mapString;

    long* _ptrLong_unlocked(const SdbKey::SdbKey key, long _default);

    String* _ptrString_unlocked(const SdbKey::SdbKey key, String& _default);
};

//
#endif // __INC_SDB_DATA_STORE_H

