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
        _mod_name(name) {
    }

    const String& name() {
        return _mod_name;
    }

    // void enqueue(msg) { _msg_queue.add(msg); }

    virtual void onStart() {}
    
    virtual long onLoop() {
        return 1000 /*ms*/;
    }

protected:
    SdbModManager& _manager;
    const String _mod_name;
};

class SdbModTask : public SdbMod {
public:
    SdbModTask(SdbModManager& manager, const String& mod_name, const String& task_name, SdbPriority::SdbPriority priority) :
        SdbMod(manager, mod_name),
        _mod_task_impl(this, task_name, priority)
    { }

    void startTask() {
        _mod_task_impl.start();
    }

    bool isTaskStarted() {
        return _mod_task_impl.isStarted();
    }

    virtual void onTaskRun() = 0;

private:
    class _SdbModTaskImpl : public SdbTask {
    public:
        _SdbModTaskImpl(SdbModTask* mod_task, const String& task_name, SdbPriority::SdbPriority priority) :
            SdbTask(task_name, priority),
            _mod_task(mod_task) {            
        }

        void onRun() override {
            _mod_task->onTaskRun();
        }
    private:
        SdbModTask* _mod_task;
    };

    _SdbModTaskImpl _mod_task_impl;
};

#endif // __INC_SDB_MOD_H
