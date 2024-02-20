#pragma once

#include "queue.h"

typedef QueueHandle_t SemaphoreHandle_t;

SemaphoreHandle_t xSemaphoreCreateRecursiveMutex() {
    return new QueueDefinition();
}

BaseType_t xSemaphoreTakeRecursive(SemaphoreHandle_t xMutex,
                             TickType_t xBlockTime) {
    return pdTRUE;
}

BaseType_t xSemaphoreGiveRecursive(SemaphoreHandle_t xMutex) {
    return pdTRUE;
}
