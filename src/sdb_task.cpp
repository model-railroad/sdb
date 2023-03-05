#include "sdb_task.h"

void SdbTask::start() {
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

void SdbTask::_entryPoint(void *taskParameters) {
    SdbTask* task = (SdbTask*)taskParameters;
    DEBUG_PRINTF( ("[%s] Task running on Core %d\n",  task->_taskName.c_str(), xPortGetCoreID()) );
    task->onRun();
}
