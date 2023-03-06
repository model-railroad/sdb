#ifndef __INC_SDB_MOD_H
#define __INC_SDB_MOD_H

class SdbModManager;

#include "common.h"
#include "sdb_lock.h"
#include "sdb_mod_manager.h"
#include "sdb_task.h"
#include <vector>

namespace SdbEvent {
    enum SdbEvent {
        Empty,
        DisplayWifiAP,
        DisplaySensor,
    };
}

class SdbMod {
public:
    SdbMod(SdbModManager& manager, const String& name) :
        _manager(manager),
        _modName(name),
        _eventLock(name)
    { }

    const String& name() {
        return _modName;
    }

    virtual void onStart() {}
    
    virtual long onLoop() {
        return 1000 /*ms*/;
    }

    void queueEvent(const SdbEvent::SdbEvent event) {
        SdbMutex eventMutex(_eventLock);
        _events.push_back(event);
    }

protected:
    SdbModManager& _manager;
    const String _modName;
    std::vector<SdbEvent::SdbEvent> _events;
    SdbLock _eventLock;

    const SdbEvent::SdbEvent dequeueEvent() {
        SdbMutex eventMutex(_eventLock);
        if (_events.empty()) {
            return SdbEvent::Empty;
        }
        auto result = _events.front();
        _events.erase(_events.begin());
        return result;
    }
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
