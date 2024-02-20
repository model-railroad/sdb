/** Mock version of the Arduino String class implemented over std::string. */
#pragma once

#include <string>

class String {
public:
    String(const char *str = "") : _str(str) {}

    const char *c_str() const { return _str.c_str(); }

private:
    std::string _str;
};
