#ifndef __INC_SDB_TASK_H
#define __INC_SDB_TASK_H

#include "common.h"
#include "sdb_lock.h"

namespace SdbPriority {
    enum SdbPriority {
        Idle = 0,
        MainLoop = 1,
        Display = 2,
        Logic = 3,
        Sensor = 4,
        Max = configMAX_PRIORITIES
    };
}

class SdbTask {
public:
    SdbTask(const String& name, SdbPriority::SdbPriority priority) :
        _taskName(name),
        _handle(NULL),
        _priority(priority)
    { }

    void start();

    bool isStarted() {
        return _handle != NULL;
    }

    virtual void onRun() = 0;

protected:
    const String _taskName;
    const SdbPriority::SdbPriority _priority;
    TaskHandle_t _handle;

private:
    static void _entryPoint(void *taskParameters);
};

#endif // __INC_SDB_TASK_H
