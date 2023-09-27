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
#define PANIC_PRINTLN(x)  { Serial.println( F(x) ) ; sdbPanic(NULL); }

#define CHECK_ESP_OK(err) ( (err) == ESP_OK )

#define PANIC_ESP_PRINTLN(err, x) { if (!CHECK_ESP_OK(err)) \
    { Serial.println( F(x) ); \
      Serial.printf("\nESP Error %02d: %s\n", (err), esp_err_to_name(err)); \
      sdbPanic(NULL); } }

#define DEBUG_ESP_PRINTLN(err, x) { if (!CHECK_ESP_OK(err)) \
    { Serial.println( F(x) ); \
      Serial.printf("\nESP Error %02d: %s\n", (err), esp_err_to_name(err)); } }

#define ERROR_ESP_PRINTLN(err, x) { if (!CHECK_ESP_OK(err)) \
    { Serial.println( F(x) ); \
      Serial.printf("\nESP Error %02d: %s\n", (err), esp_err_to_name(err)); } }

// CPU affinity for ESP32
#define PRO_CPU 0       // Wifi
#define APP_CPU 1       // Main app

// In main INO... a global panic method.
void sdbPanic(char* msg = NULL);

#endif // __INC_COMMON_H
