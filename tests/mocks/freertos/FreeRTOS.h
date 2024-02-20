#pragma once

#include "WString.h"


typedef signed char int8_t ;
typedef signed int int16_t;
typedef signed long int32_t;
typedef unsigned char uint8_t ;
typedef unsigned int uint16_t;
typedef unsigned long uint32_t;

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
