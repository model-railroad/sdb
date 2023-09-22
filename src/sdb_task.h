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

#ifndef __INC_SDB_TASK_H
#define __INC_SDB_TASK_H

#include "common.h"
#include "sdb_lock.h"

namespace SdbPriority {
    enum SdbPriority {
        Idle = 0,
        MainLoop = 1,
        Display = 2,
        Network = 3,
        Logic = 4,
        Sensor = 5,
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

    void start() {
        if (xTaskCreatePinnedToCore(
                _entryPoint,    // pvTaskCode,
                _taskName.c_str(),  // pcName (16 char max),
                4096,           // usStackDepth in bytes
                this,           // pvParameters,
                _priority,      // uxPriority, from 0 to configMAX_PRIORITIES
                &_handle,       // pvCreatedTask
                APP_CPU /*tskNO_AFFINITY*/) != pdPASS) {
            ERROR_PRINTF( ("[%s] FATAL: xTaskCreate failed.\n", _taskName.c_str()) );
        } else {
            DEBUG_PRINTF( ("[%s]  Task created == %p\n", _taskName.c_str(), _handle) );
        }
    }

    bool isStarted() {
        return _handle != NULL;
    }

    virtual void onRun() = 0;

protected:
    const String _taskName;
    const SdbPriority::SdbPriority _priority;
    TaskHandle_t _handle;

private:
    static void _entryPoint(void *taskParameters) {
        SdbTask* task = (SdbTask*)taskParameters;
        DEBUG_PRINTF( ("[%s] Task running on Core %d\n",  task->_taskName.c_str(), xPortGetCoreID()) );
        task->onRun();
    }
};

#endif // __INC_SDB_TASK_H
