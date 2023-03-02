#ifndef __INC_SDB_LOCK_H
#define __INC_SDB_LOCK_H

#include "common.h"
#include <functional>

class SdbLock {
public:
    SdbLock(String name) :
        _name(name),
        _is_locked(false)
    { }

    void execute(const std::function<void()> lambda) {
        try {
            acquire();
            lambda();
        } catch (...) {
            DEBUG_PRINTF( ("Print something") );
        }
        release();
    }

    void acquire() {
        if (!_is_locked) {
            _is_locked = true;
        }
    }

    void release() {
        if (_is_locked) {
            _is_locked = false;
        }
    }

private:
    String _name;
    bool _is_locked;

};

#endif // __INC_SDB_LOCK_H
