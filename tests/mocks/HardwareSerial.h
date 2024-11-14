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

#include <stdio.h>
#include <stdarg.h>

#define F(x) (x)

class Print {
public:
    size_t printf(const char *format, ...) {
        va_list argptr;
        va_start(argptr, format);
        int result = vfprintf(stderr, format, argptr);
        va_end(argptr);
        return result;
    }

    size_t println(const char *str) {
        return fprintf(stderr, "%s\n", str);
    }

    void begin(
            unsigned long baud
            // uint32_t config = SERIAL_8N1,
            // int8_t rxPin = -1,
            // int8_t txPin = -1,
            // bool invert = false,
            // unsigned long timeout_ms = 20000UL,
            // uint8_t rxfifo_full_thrhd = 112
    ) { }

    void setDebugOutput(bool) { };
};

class Stream : public Print {};

class HardwareSerial: public Stream {};

HardwareSerial Serial;

