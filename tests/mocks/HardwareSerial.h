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
};

class Stream : public Print {};

class HardwareSerial: public Stream {};

HardwareSerial Serial;

