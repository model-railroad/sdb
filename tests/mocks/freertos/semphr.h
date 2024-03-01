/*
 * Project: Software Defined Blocks
 * Copyright (C) 2024 alf.labs gmail com.
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
