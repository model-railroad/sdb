#ifndef __INC_SDB_MOD_H
#define __INC_SDB_MOD_H

class SdbModManager;

#include "common.h"
#include "sdb_mod_manager.h"
#include "sdb_task.h"

class SdbMod {
public:
    SdbMod(SdbModManager& manager, const String& name) :
        _manager(manager),
        _modName(name) {
    }

    const String& name() {
        return _modName;
    }

    // void enqueue(msg) { _msg_queue.add(msg); }

    virtual void onStart() {}
    
    virtual long onLoop() {
        return 1000 /*ms*/;
    }

protected:
    SdbModManager& _manager;
    const String _modName;
};

class SdbModTask : public SdbMod {
public:
    SdbModTask(SdbModManager& manager, const String& modName, const String& taskName, SdbPriority::SdbPriority priority) :
        SdbMod(manager, modName),
        _modTaskImpl(this, taskName, priority)
    { }

    void startTask() {
        _modTaskImpl.start();
    }

    bool isTaskStarted() {
        return _modTaskImpl.isStarted();
    }

    virtual void onTaskRun() = 0;

private:
    class _SdbModTaskImpl : public SdbTask {
    public:
        _SdbModTaskImpl(SdbModTask* mod_task, const String& taskName, SdbPriority::SdbPriority priority) :
            SdbTask(taskName, priority),
            _modTask(mod_task) {            
        }

        void onRun() override {
            _modTask->onTaskRun();
        }
    private:
        SdbModTask* _modTask;
    };

    _SdbModTaskImpl _modTaskImpl;
};

#endif // __INC_SDB_MOD_H
