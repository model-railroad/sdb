/** Mock version of the Arduino String class implemented over std::string. */
#pragma once

#include <string>

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2


class String {
public:
    String(const char *str = "") : _str(str) {}
    String(int val, int base) : _str("TODO convert int to base") {}

    const char *c_str() const { return _str.c_str(); }

    bool equals(const String &s) const {
        return _str == s._str;
    }

    bool equals(const char *cstr) const {
        return _str == cstr;
    }

    bool operator ==(const String &rhs) const { return equals(rhs); }
    bool operator ==(const char *cstr) const { return equals(cstr); }

    bool operator <(const String &rhs) const { return _str < rhs._str; }
    bool operator >(const String &rhs) const { return _str > rhs._str; }
    bool operator <=(const String &rhs) const { return _str <= rhs._str; }
    bool operator >=(const String &rhs) const { return _str >= rhs._str; }


private:
    std::string _str;
};
