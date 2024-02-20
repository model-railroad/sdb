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
    String(int val) : String(val, 10) { }
    String(int val, int radix) {
        char buf[1024];
        _str = itoa(val, buf, radix);
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

    void trim() {
        ltrim(_str);
        rtrim(_str);
    }

    bool isEmpty() const { return _str.empty(); }

    int toInt() const { return std::stoi(_str); }

    int length() const { return _str.length(); }
    
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
