#ifndef __INC_SDB_MOD_H
#define __INC_SDB_MOD_H

class SdbModManager;

#include "common.h"
#include "sdb_mod_manager.h"

class SdbMod {
public:
    SdbMod(SdbModManager& manager, const String& name) :
        _manager(manager),
        _name(name) {
    }

    const String& name() {
        return _name;
    }

    // void enqueue(msg) { _msg_queue.add(msg); }

    virtual void onStart() {}
    
    virtual long onLoop() {
        return 1000 /*ms*/;
    }

protected:
    SdbModManager& _manager;
    const String _name;
};

#endif // __INC_SDB_MOD_H
