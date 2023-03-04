#ifndef __INC_COMMON_H
#define __INC_COMMON_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <HardwareSerial.h>

#if !defined(ESP32)
    #warning ESP32 is not defined. Defining it here.
    #define ESP32 1
#endif

#define MS_TO_TICKS(ms) ((ms) / portTICK_PERIOD_MS)
#define rtDelay(ms) { vTaskDelay( MS_TO_TICKS(ms) ); } // delay in mS

#define VERBOSE_PRINTF(x) { /*Serial.printf x ; */ }
#define DEBUG_PRINTF(x)   { Serial.printf x ; }
#define ERROR_PRINTF(x)   { Serial.printf x ; }
#define PANIC_PRINTF(x)   { Serial.printf x ; sdbPanic(NULL); }

// CPU affinity for ESP32
#define PRO_CPU 0       // Wifi
#define APP_CPU 1       // Main app

// In main INO... a global panic method.
void sdbPanic(char* msg = NULL);

#endif // __INC_COMMON_H
