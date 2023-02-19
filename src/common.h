#ifndef __INC_COMMON_H
#define __INC_COMMON_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <HardwareSerial.h>

#define MS_TO_TICKS(ms) ((ms) / portTICK_PERIOD_MS)
#define rtDelay(ms) { vTaskDelay( MS_TO_TICKS(ms) ); } // delay in mS

#define VERBOSE_PRINTF(x) { /*Serial.printf x ; */ }
#define DEBUG_PRINTF(x)   { Serial.printf x ; }
#define ERROR_PRINTF(x)   { Serial.printf x ; }

// CPU affinity for ESP32
#define PRO_CPU 0
#define APP_CPU 1

//-- In main INO
//-- bool is_ota_updating();

#endif // __INC_COMMON_H
