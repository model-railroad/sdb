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

#ifndef INC_SDB_DATA_STORE_H
#define INC_SDB_DATA_STORE_H

#include "common.h"
#include "sdb_lock.h"
#include <unordered_map>
#include <nvs_flash.h>
#include <nvs.h>
#include <nvs_handle.hpp>

namespace SdbKey {
    /// Keys for SdbDataStore
    enum SdbKey {
        /// In-memory keys, not backed into NVS.
        InMemoryStart = 0,
        SoftApIpStr,
        TofDistanceMmLong,

        /// NVS -- WARNING: All NVS keys must be considered constants;
        /// the enum values should not change across updates.
        NvsStart        = 0x8000,
        WifiSsidStr     = 0x8001,
        WifiPassStr     = 0x8002,
    };
}

/**
 * An in-memory data store of key/value pairs.
 *
 * Values accept 3 types: long, String, and blobs.
 * The key should be kep unique across all data types.
 * Strings are copied in the in-memory maps.
 * Blobs are allocated by the caller, and should be freed by the callers accordingly.
 *
 * Keys above SdkKey::NvsStart are also stored in the ESP32 Non-Volatile Storage (NVS).
 * When read, the in-memory value prevails; the NVS is only read if there is no initial
 * copy in memory. When writing, both the in-memory and the NVS versions are updated.
 */
class SdbDataStore {
public:
    SdbDataStore() :
        _lock("LockData")
    { }

    SdbLock& lock() {
        return _lock;
    }

    void onStart() {
        // Initialize NVS
        esp_err_t err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            // From sample code: NVS partition was truncated and needs to be erased.
            // Then retry nvs_flash_init
            err = nvs_flash_erase();
            DEBUG_ESP_PRINTLN(err, "NVS flash erase failed");
            if (err == ESP_OK) {
                err = nvs_flash_init();
            }
        }
        PANIC_ESP_PRINTLN(err, "NVS init failed.");
    }

    /// Inserts this long value at the given key.
    long putLong(const SdbKey::SdbKey key, long value) {
        SdbMutex autoMutex(_lock);
        _mapLong[key] = value;

        if (key > SdbKey::NvsStart) {
            esp_err_t err;
            std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle("sdb", NVS_READWRITE, &err);
            PANIC_ESP_PRINTLN(err, "NVS open failed.");
            String nvsKey(key, HEX);
            err = handle->set_item(nvsKey.c_str(), (int32_t)value);
            DEBUG_ESP_PRINTLN(err, "NVS write failed");
            err = handle->commit();
            DEBUG_ESP_PRINTLN(err, "NVS commit failed");
        }

        return value;
    }

    /// Retrieves the long value at the given key, or returns the default value.
    long getLong(const SdbKey::SdbKey key, long _default) {
        SdbMutex autoMutex(_lock);

        if (key > SdbKey::NvsStart && !loadedFromNvs(key)) {
            _mapNvs[key] = true;
            esp_err_t err;
            std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle("sdb", NVS_READWRITE, &err);
            PANIC_ESP_PRINTLN(err, "NVS open failed.");
            String nvsKey(key, HEX);
            int32_t data = 0;
            err = handle->get_item(nvsKey.c_str(), data);
            DEBUG_ESP_PRINTLN(err, "NVS get string size failed");
            if (CHECK_ESP_OK(err)) {
                _mapLong[key] = data;
                return data;
            }
        }

        auto kvKeyLong = _mapLong.find(key);
        if (kvKeyLong == _mapLong.end()) {
            return _default;
        } else {
            return kvKeyLong->second;
        }
    }

    /// Inserts this string value at the given key. A copy is made.
    void putString(const SdbKey::SdbKey key, const String& value) {
        SdbMutex autoMutex(_lock);
        _mapString[key] = value;

        if (key > SdbKey::NvsStart) {
            esp_err_t err;
            std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle("sdb", NVS_READWRITE, &err);
            PANIC_ESP_PRINTLN(err, "NVS open failed.");
            String nvsKey(key, HEX);
            err = handle->set_string(nvsKey.c_str(), value.c_str());
            DEBUG_ESP_PRINTLN(err, "NVS write failed");
            err = handle->commit();
            DEBUG_ESP_PRINTLN(err, "NVS commit failed");
        }
    }

    /// Retrieves the string reference at the given key, or returns the default value.
    const String& getString(const SdbKey::SdbKey key, const String& _default) {
        SdbMutex autoMutex(_lock);

        if (key > SdbKey::NvsStart && !loadedFromNvs(key)) {
            _mapNvs[key] = true;
            esp_err_t err;
            std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle("sdb", NVS_READWRITE, &err);
            PANIC_ESP_PRINTLN(err, "NVS open failed.");
            String nvsKey(key, HEX);
            size_t size = 0;
            err = handle->get_item_size(nvs::ItemType::SZ, nvsKey.c_str(), size);
            DEBUG_ESP_PRINTLN(err, "NVS get string size failed");
            if (CHECK_ESP_OK(err) && size > 0) {                
                char* dest = (char*)malloc(size); // TBD use a shared_ptr(malloc,free)
                err = handle->get_string(nvsKey.c_str(), dest, size);
                DEBUG_ESP_PRINTLN(err, "NVS get string failed");
                if (CHECK_ESP_OK(err)) {
                    _mapString[key] = String(dest);
                    // String dest has limited scope; don't return a reference to it.
                }
                free(dest);
            }
        }

        auto kvKeyStr = _mapString.find(key);
        if (kvKeyStr == _mapString.end()) {
            return _default;
        } else {
            return kvKeyStr->second;
        }
    }

private:
    SdbLock _lock;
    /// An unordered map of SdbKey(as int) to long values.
    std::unordered_map<SdbKey::SdbKey, long, std::hash<int>> _mapLong;
    /// An unordered map of SdbKey(as int) to String values.
    std::unordered_map<SdbKey::SdbKey, String, std::hash<int>> _mapString;
    /// Map of keys loaded from NVS
    std::unordered_map<SdbKey::SdbKey, bool, std::hash<int>> _mapNvs;

    bool loadedFromNvs(const SdbKey::SdbKey key) {
        auto loaded = _mapNvs.find(key);
        return loaded != _mapNvs.end();
    }
};


#endif // INC_SDB_DATA_STORE_H

