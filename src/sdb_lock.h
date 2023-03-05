#ifndef __INC_SDB_LOCK_H
#define __INC_SDB_LOCK_H

#include "common.h"
#include <assert.h>
#include <freertos/semphr.h>
#include <functional>

class SdbLock {
public:
    SdbLock(String name) :
        _name(name),
        _handle(xSemaphoreCreateRecursiveMutex())
    {
        assert(_handle != NULL);
    }

    void execute(const std::function<void()> lambda);

    void acquire();

    void release();

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
