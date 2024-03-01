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

#include <stdint.h>

// -- ESP IDF stuff

typedef int esp_err_t;

#define ESP_OK 0
#define ESP_FAIL -1

#define ESP_ERR_NO_MEM              0x101
#define ESP_ERR_INVALID_ARG         0x102
#define ESP_ERR_INVALID_STATE       0x103
#define ESP_ERR_INVALID_SIZE        0x104
#define ESP_ERR_NOT_FOUND           0x105
#define ESP_ERR_NOT_SUPPORTED       0x106
#define ESP_ERR_TIMEOUT             0x107
#define ESP_ERR_INVALID_RESPONSE    0x108
#define ESP_ERR_INVALID_CRC         0x109
#define ESP_ERR_INVALID_VERSION     0x10A
#define ESP_ERR_INVALID_MAC         0x10B
#define ESP_ERR_NOT_FINISHED        0x10C

const char * esp_err_to_name(esp_err_t err) { return "error"; }

#define configTICK_RATE_HZ 1000
#define portTICK_PERIOD_MS ( ( TickType_t ) 1000 / configTICK_RATE_HZ )

void vTaskDelay(unsigned long ticks) { }

// -- Arduino stuff

#define BUILTIN_LED 25

unsigned long millis() { return 42UL; }
void delay(unsigned long millis) { /* no-op */ }

#define LOW               0x0
#define HIGH              0x1

//#define INPUT             0x01    -- this conflicts with mingw/.../winuser.h
#define OUTPUT            0x03
#define PULLUP            0x04
#define INPUT_PULLUP      0x05
#define PULLDOWN          0x08
#define INPUT_PULLDOWN    0x09
#define OPEN_DRAIN        0x10
#define OUTPUT_OPEN_DRAIN 0x12
#define ANALOG            0xC0

void pinMode(uint8_t pin, uint8_t mode) { }
void digitalWrite(uint8_t pin, uint8_t val) { }
int digitalRead(uint8_t pin) { return 42; }
