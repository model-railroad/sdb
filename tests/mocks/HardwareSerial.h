#pragma once

#include <stdio.h>
#include <stdarg.h>

class Print {
public:
    size_t printf(const char *format, ...) {
        va_list argptr;
        va_start(argptr, format);
        int result = vfprintf(stderr, format, argptr);
        va_end(argptr);
        return result;
    }
};

class Stream : public Print {};

class HardwareSerial: public Stream {};

HardwareSerial Serial;

