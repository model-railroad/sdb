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

module;

#include "common.h"
#include <assert.h>
#include <freertos/semphr.h>
#include <functional>

//---------------
export module SDB.Lock;

export class SdbLock {
public:
    SdbLock(const String& name) :
        _name(name),
        _handle(xSemaphoreCreateRecursiveMutex())
    {
        assert(_handle != NULL);
    }

    void execute(const std::function<void()> lambda) {
        try {
            acquire();
            lambda();
        } catch (...) {
            DEBUG_PRINTF( ("[%s] execute failed", _name.c_str()) );
        }
        release();
    }

    void acquire() {
        #if INCLUDE_vTaskSuspend != 1
            #error INCLUDE_vTaskSuspend must be 1 for portMAX_DELAY to be infinite.
        #endif
        // TODO consider not using an infinite timeout. Instead use a long one,
        // e.g. one second, then busy loop, and panic if the wait is above a
        // significant threshold (for debugging purposes).
        // Or create an acquire()-->false method, or throw.
        if (xSemaphoreTakeRecursive(_handle, portMAX_DELAY) != pdTRUE) {
            PANIC_PRINTF( ("[%s] acquire failed", _name.c_str()) );
        }
    }

    void release() {
        if (xSemaphoreGiveRecursive(_handle) != pdTRUE) {
            PANIC_PRINTF( ("[%s] release failed", _name.c_str()) );
        }
    }

private:
    const String _name;
    const SemaphoreHandle_t _handle;
};

/// A mutex is a lock that auto-acquires and auto-releases on scope.
export class SdbMutex {
public:
    SdbMutex(SdbLock& lock) : _lock(lock) {
        _lock.acquire();
    }

    ~SdbMutex() {
        _lock.release();
    }

private:
    SdbLock& _lock;
};

