/*
 * Project: Software Defined Blocks
 * Copyright (C) 2023 alf.labs gmail com.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INC_SDB_MOD_H
#define INC_SDB_MOD_H

class SdbModManager;

#include "common.h"
#include "sdb_lock.h"
#include "sdb_mod_manager.h"
#include "sdb_task.h"
#include <memory>
#include <vector>

namespace SdbEvent {
    enum Type {
        Empty,
        /// Data: None.
        DisplayWifiAP,
        /// Data: None.
        DisplayWifiSTA,
        /// Data: None.
        DisplaySensor,
        /// Data: state(bool), block(String).
        BlockChanged,
    };

    class SdbEvent {
    public:
     SdbEvent()
         : type(Empty), state(false), data(nullptr) {}

     SdbEvent(Type type) //NOLINT (we want the implicit constructor)
         : type(type), state(false), data(nullptr) {}

     SdbEvent(Type type, bool state, const String* data)
         : type(type), state(state), data(data) {}

        bool operator ==(const SdbEvent &rhs) const {
         return type == rhs.type
             && state == rhs.state
             && data == rhs.data;   // Note: String *pointer* equality
     }

        Type type;
        bool state;
        const String* data;
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
    
    virtual millis_t onLoop() {
        return 1000 /*ms*/;
    }

    void queueEvent(std::unique_ptr<SdbEvent::SdbEvent> event) {
        SdbMutex eventMutex(_eventLock);
        _events.push_back(std::move(event));
    }

protected:
    SdbModManager& _manager;
    const String _modName;
    std::vector<std::unique_ptr<SdbEvent::SdbEvent>> _events;
    SdbLock _eventLock;

    bool hasEvents() {
        SdbMutex eventMutex(_eventLock);
        return !_events.empty();
    }

    /// Returns the oldest event or nullptr if there are not queued.
    std::unique_ptr<SdbEvent::SdbEvent> dequeueEvent() {
        SdbMutex eventMutex(_eventLock);
        if (_events.empty()) {
            return { nullptr }; // Note: equivalent to std::unique_ptr<SdbEvent::SdbEvent>(nullptr);
        }
        auto result = std::move(_events.front());
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

#endif // INC_SDB_MOD_H
