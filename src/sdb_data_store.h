#ifndef __INC_SDB_DATA_STORE_H
#define __INC_SDB_DATA_STORE_H

#include "common.h"
#include "sdb_lock.h"
#include <unordered_map>

namespace SdbKey {
    enum SdbKey {
        Empty,
        TofDistanceMM,
        SoftAPIP,
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

    void putLong(const SdbKey::SdbKey key, long value) {
        SdbMutex autoMutex(_lock);
        long* ptr = _ptrLong_unlocked(key, value);
        *ptr = value;
    }

    long getLong(const SdbKey::SdbKey key, long _default) {
        SdbMutex autoMutex(_lock);
        return *_ptrLong_unlocked(key, _default);
    }

    long* ptrLong(const SdbKey::SdbKey key, long _default) {
        SdbMutex autoMutex(_lock);
        return _ptrLong_unlocked(key, _default);
    }

    void putString(const SdbKey::SdbKey key, const String& value) {
        SdbMutex autoMutex(_lock);
        String* ptr = ptrString(key, value);
        *ptr = value;
    }

    String& getString(const SdbKey::SdbKey key, const String& _default) {
        SdbMutex autoMutex(_lock);
        return *ptrString(key, _default);
    }

    String* ptrString(const SdbKey::SdbKey key, const String& _default) {
        SdbMutex autoMutex(_lock);
        return _ptrString_unlocked(key, _default);
    }

private:
    SdbLock _lock;
    // An unordered map of SdbKey(as int) to long values.
    std::unordered_map<SdbKey::SdbKey, long*, std::hash<int>> _mapLong;
    // An unordered map of SdbKey(as int) to String values.
    std::unordered_map<SdbKey::SdbKey, String*, std::hash<int>> _mapString;

    long* _ptrLong_unlocked(const SdbKey::SdbKey key, long _default) {
        auto keyValue = _mapLong.find(key);
        if (keyValue == _mapLong.end()) {
            long* val = (long*) calloc(1, sizeof(long));
            *val = _default;
            _mapLong[key] = val;
            return val;
        } else {
            return keyValue->second;
        }
    }

    String* _ptrString_unlocked(const SdbKey::SdbKey key, const String& _default) {
        auto keyValue = _mapString.find(key);
        if (keyValue == _mapString.end()) {
            String* val = (String*) calloc(1, sizeof(String));
            *val = _default;
            _mapString[key] = val;
            return val;
        } else {
            return keyValue->second;
        }
    }
};

//
#endif // __INC_SDB_DATA_STORE_H

