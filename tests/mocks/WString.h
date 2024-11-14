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

/** Mock version of the Arduino String class implemented over std::string. */
#pragma once

#include <algorithm>
#include <string>

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2


class String {
public:
    String(const char *str = "") : _str(str) {}
    String(int val, int base = 10) {
        char buf[1024];
#if defined(itoa)
        _str = itoa(val, buf, base);
#else
        assert(base == 10); // lazy mock implementation
        _str = std::to_string(val);
#endif
    }

    const char *c_str() const { return _str.c_str(); }

    bool equals(const String & s) const { return _str == s._str; }
    bool equals(const char *cstr) const { return _str == cstr; }
    bool operator ==(const String &rhs) const { return equals(rhs); }
    bool operator ==(const char  *cstr) const { return equals(cstr); }
    bool operator !=(const String &rhs) const { return !equals(rhs); }
    bool operator !=(const char  *cstr) const { return !equals(cstr); }

    bool operator < (const String &rhs) const { return _str < rhs._str; }
    bool operator > (const String &rhs) const { return _str > rhs._str; }
    bool operator <=(const String &rhs) const { return _str <= rhs._str; }
    bool operator >=(const String &rhs) const { return _str >= rhs._str; }

    String & operator +=(const String str) {
        _str += str._str;
        return (*this);
    }
    String & operator +=(const char *cstr) {
        _str += cstr;
        return (*this);
    }

    char charAt(unsigned int loc) const {
        return _str[loc];
    }

    void trim() {
        ltrim(_str);
        rtrim(_str);
    }

    bool isEmpty() const { return _str.empty(); }

    long toInt() const { return atol(_str.c_str()); }

    int length() const { return _str.length(); }

    // Visible for Testing
    const std::string& internal() const { return _str; }

private:
    std::string _str;

    // stackoverflow.com/questions/216823
    void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
    }

    void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }
};

// Stringification for doctest C++ framework
std::ostream& operator<<(std::ostream& os, const String& value) {
    os << value.internal();
    return os;
}
