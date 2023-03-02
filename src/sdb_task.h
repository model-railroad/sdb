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
    SdbTask(String name, SdbPriority::SdbPriority priority) :
        _name(name),
        _handle(NULL),
        _priority(priority)
    { }

    void start() {
        if (xTaskCreatePinnedToCore(
                _entryPoint,    // pvTaskCode,
                _name.c_str(),  // pcName (16 char max),
                4096,           // usStackDepth in bytes
                this,           // pvParameters,
                _priority,      // uxPriority, from 0 to configMAX_PRIORITIES
                &_handle,       // pvCreatedTask
                APP_CPU /*tskNO_AFFINITY*/) != pdPASS) {
            ERROR_PRINTF( ("[%s] FATAL: xTaskCreate failed.\n", _name.c_str()) );
        } else {
            DEBUG_PRINTF( ("[%s]  Task created == %p\n", _name.c_str(), _handle) );
        }
    }

    bool isStarted() {
        return _handle != NULL;
    }

    virtual void onRun() = 0;

private:
    String _name;
    SdbPriority::SdbPriority _priority;
    TaskHandle_t _handle;

    static void _entryPoint(void *taskParameters) {
        SdbTask* task = (SdbTask*)taskParameters;
        DEBUG_PRINTF( ("[%s] Task running on Core %d\n",  task->_name.c_str(), xPortGetCoreID()) );
        task->onRun();
    }
};

#endif // __INC_SDB_TASK_H
