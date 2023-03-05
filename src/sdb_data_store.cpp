#include "sdb_data_store.h"

void SdbDataStore::putLong(const SdbKey::SdbKey key, long value) {
    SdbMutex autoMutex(_lock);
    long* ptr = _ptrLong_unlocked(key, value);
    *ptr = value;
}

long SdbDataStore::getLong(const SdbKey::SdbKey key, long _default) {
    SdbMutex autoMutex(_lock);
    return *_ptrLong_unlocked(key, _default);
}

long* SdbDataStore::ptrLong(const SdbKey::SdbKey key, long _default) {
    SdbMutex autoMutex(_lock);
    return _ptrLong_unlocked(key, _default);
}

void SdbDataStore::putString(const SdbKey::SdbKey key, String& value) {
    SdbMutex autoMutex(_lock);
    String* ptr = ptrString(key, value);
    *ptr = value;
}

String& SdbDataStore::getString(const SdbKey::SdbKey key, String& _default) {
    SdbMutex autoMutex(_lock);
    return *ptrString(key, _default);
}

String* SdbDataStore::ptrString(const SdbKey::SdbKey key, String& _default) {
    SdbMutex autoMutex(_lock);
    return _ptrString_unlocked(key, _default);
}

long* SdbDataStore::_ptrLong_unlocked(const SdbKey::SdbKey key, long _default) {
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

String* SdbDataStore::_ptrString_unlocked(const SdbKey::SdbKey key, String& _default) {
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

