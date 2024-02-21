#pragma once

typedef void* TaskHandle_t;
typedef void (* TaskFunction_t)( void * );

#define tskIDLE_PRIORITY ((UBaseType_t) 0U)

BaseType_t xTaskCreatePinnedToCore( TaskFunction_t pvTaskCode,
                                    const char * const pcName,
                                    const uint32_t usStackDepth,
                                    void * const pvParameters,
                                    UBaseType_t uxPriority,
                                    TaskHandle_t * const pvCreatedTask,
                                    const BaseType_t xCoreID) {
    return pdTRUE;
}

BaseType_t xPortGetCoreID(void) {
    return APP_CPU;
}
