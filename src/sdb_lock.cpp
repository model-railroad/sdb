#include "sdb_lock.h"

void SdbLock::execute(const std::function<void()> lambda) {
    try {
        acquire();
        lambda();
    } catch (...) {
        DEBUG_PRINTF( ("[%s] execute failed", _name.c_str()) );
    }
    release();
}

void SdbLock::acquire() {
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

void SdbLock::release() {
    if (xSemaphoreGiveRecursive(_handle) != pdTRUE) {
        PANIC_PRINTF( ("[%s] release failed", _name.c_str()) );
    }
}

