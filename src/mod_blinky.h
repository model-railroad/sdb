#ifndef __INC_SDB_MOD_BLINKY_H
#define __INC_SDB_MOD_BLINKY_H

#include "common.h"
#include "sdb_lock.h"
#include "sdb_mod.h"

class SdbModBlinky : public SdbModTask {
public:
    SdbModBlinky(SdbModManager& manager) :
        SdbModTask(manager, "ld", "TaskBlinky", SdbPriority::Sensor),
        _ioLock(manager.ioLock())
    { }

    void onStart() override;

    long onLoop() override;

private:
    SdbLock& _ioLock;

    void onTaskRun() override;
};


#endif // __INC_SDB_MOD_BLINKY_H
