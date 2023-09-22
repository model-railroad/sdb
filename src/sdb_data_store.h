/*
 * Project: Software Defined Blocks
 * Copyright (C) 2023 alf.labs gmail com.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __INC_SDB_DATA_STORE_H
#define __INC_SDB_DATA_STORE_H

#include "common.h"
#include "sdb_lock.h"
#include <unordered_map>

namespace SdbKey {
    enum SdbKey {
        Empty = 0,
        SoftApIpStr = 1,
        // Sensors
        TofDistanceMmLong = 0x100,
        // NVS
        NvsStart = 0x8000,
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

    long putLong(const SdbKey::SdbKey key, long value) {
        SdbMutex autoMutex(_lock);
        _mapLong[key] = value;
        return value;
    }

    long getLong(const SdbKey::SdbKey key, long _default) {
        SdbMutex autoMutex(_lock);
        auto kvKeyLong = _mapLong.find(key);
        if (kvKeyLong == _mapLong.end()) {
            return _default;
        } else {
            return kvKeyLong->second;
        }
    }

    void putString(const SdbKey::SdbKey key, const String& value) {
        SdbMutex autoMutex(_lock);
        _mapString[key] = value;
    }

    const String& getString(const SdbKey::SdbKey key, const String& _default) {
        SdbMutex autoMutex(_lock);
        auto kvKeyStr = _mapString.find(key);
        if (kvKeyStr == _mapString.end()) {
            return _default;
        } else {
            return kvKeyStr->second;
        }
    }

private:
    SdbLock _lock;
    // An unordered map of SdbKey(as int) to long values.
    std::unordered_map<SdbKey::SdbKey, long, std::hash<int>> _mapLong;
    // An unordered map of SdbKey(as int) to String values.
    std::unordered_map<SdbKey::SdbKey, String, std::hash<int>> _mapString;
};


#endif // __INC_SDB_DATA_STORE_H

