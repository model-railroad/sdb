#pragma once

#include "queue.h"

typedef QueueHandle_t SemaphoreHandle_t;

// Visible for Testing
SemaphoreHandle_t gLastSemaphore = nullptr;

SemaphoreHandle_t xSemaphoreCreateRecursiveMutex() {
    auto s = new QueueDefinition();
    gLastSemaphore = s;
    return s;
}

BaseType_t xSemaphoreTakeRecursive(SemaphoreHandle_t xMutex,
                             TickType_t xBlockTime) {
    xMutex->counter++;
    gLastSemaphore = xMutex;
    return pdTRUE;
}

BaseType_t xSemaphoreGiveRecursive(SemaphoreHandle_t xMutex) {
    xMutex->counter--;
    gLastSemaphore = xMutex;
    return pdTRUE;
}
