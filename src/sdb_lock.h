#ifndef __INC_SDB_LOCK_H
#define __INC_SDB_LOCK_H

#include "common.h"
#include <freertos/semphr.h>
#include <functional>

class SdbLock {
public:
    SdbLock(String name) :
        _name(name),
        _handle(xSemaphoreCreateMutex())
    { }

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
        if (xSemaphoreTake(_handle, portMAX_DELAY) != pdTRUE) {
            DEBUG_PRINTF( ("[%s] acquire failed", _name.c_str()) );
        }
    }

    void release() {
        xSemaphoreGive(_handle);
    }

private:
    String _name;
    SemaphoreHandle_t _handle;
};

/// A mutex is a lock that auto-acquires and auto-releases on scope.
class SdbMutex {
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

#endif // __INC_SDB_LOCK_H
