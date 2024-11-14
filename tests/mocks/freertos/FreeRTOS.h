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

#include "WString.h"
#include <stdint.h>

typedef signed char int8_t ;
typedef unsigned char uint8_t ;

typedef uint32_t TickType_t;

#define portCHAR                    int8_t
#define portFLOAT                   float
#define portDOUBLE                  double
#define portLONG                    int32_t
#define portSHORT                   int16_t
#define portSTACK_TYPE              uint8_t
#define portBASE_TYPE               int

typedef portSTACK_TYPE              StackType_t;
typedef portBASE_TYPE               BaseType_t;
typedef unsigned portBASE_TYPE      UBaseType_t;

#define pdFALSE ((BaseType_t) 0)
#define pdTRUE  ((BaseType_t) 1)
#define pdPASS  pdTRUE
#define pdFAIL  pdFALSE
#define errQUEUE_EMPTY pdFAIL
#define errQUEUE_FULL  pdFAIL

#define INCLUDE_vTaskSuspend 1
#define portMAX_DELAY (TickType_t) 0xffffffffUL

#define configMAX_PRIORITIES (25)

#define WIFI_TASK_CORE_ID 1
