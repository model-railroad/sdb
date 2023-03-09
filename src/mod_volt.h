#ifndef __INC_SDB_MOD_VOLT_H
#define __INC_SDB_MOD_VOLT_H

#include "common.h"
#include "sdb_lock.h"
#include "sdb_mod.h"

#define MOD_VOLT_NAME "vo"

class SdbModVolt : public SdbModTask {
public:
    SdbModVolt(SdbModManager& manager) :
        SdbModTask(manager, MOD_VOLT_NAME, "TaskVolt", SdbPriority::Sensor),
        _ioLock(_manager.ioLock()),
        _dataLock(_manager.dataStore().lock()),
        _sharedMV(NULL)
    { }

    void onStart() override {
        _sharedMV = _manager.dataStore().ptrLong(SdbKey::VoltageMV, 0);
        startTask();
    }

    long onLoop() override {
        return 2000;
    }

private:
    long* _sharedMV;
    SdbLock& _ioLock;
    SdbLock& _dataLock;

    void onTaskRun() {
        // TBD init
        while(true) {
            // TBD loop
            rtDelay(250);
        }
    }

    void update_data_store(long voltageMV) {
        SdbMutex dataMutex(_dataLock);
        if (*_sharedMV != voltageMV) {
            *_sharedMV = voltageMV;
        }
    }
};

#endif // __INC_SDB_MOD_VOLT_H
