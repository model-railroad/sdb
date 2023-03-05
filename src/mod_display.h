#ifndef __INC_SDB_MOD_DISPLAY_H
#define __INC_SDB_MOD_DISPLAY_H

// Important: I2C controller 0 (aka &Wire ) is used here.
//            I2C controller 1 (aka &Wire1) is used in mod_tof.
// WifiKit32 OLED I2C address is 0x3C, pins sda=4 scl=16 reset=16.

#include "common.h"
#include "sdb_lock.h"
#include "sdb_mod.h"

#define USE_DISPLAY_LIB_U8G2
#undef  USE_DISPLAY_LIB_AF_GFX
// #undef  USE_DISPLAY_LIB_U8G2
// #define USE_DISPLAY_LIB_AF_GFX

#if defined(USE_DISPLAY_LIB_U8G2)
#include <U8g2lib.h>
#elif defined(USE_DISPLAY_LIB_AF_GFX)
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#endif

#define DISPLAY_TIME_ON_MS 15*1000

class SdbModDisplay : public SdbMod {
public:
    SdbModDisplay(SdbModManager& manager) :
        SdbMod(manager, "dp"),
        _ioLock(_manager.ioLock()),
        _dataLock(_manager.dataStore().lock()),
        _sharedDistMM(NULL),
        _lastDistMM(0),
#if defined(USE_DISPLAY_LIB_U8G2)
        // U8G2 INIT -- OLED U8G2 constructor for ESP32 WIFI_KIT_32 I2C bus on I2C pins 4+15+16
        // _u8g2(U8G2_R0, /*SCL*/ 15, /*SDA*/ 4, /*RESET*/ 16), // for U8G2_SSD1306_128X64_NONAME_F_SW_I2C
        _u8g2(U8G2_R0, /*RESET*/ 16, /*SCL*/ 15, /*SDA*/ 4), // for U8G2_SSD1306_128X64_NONAME_F_HW_I2C
#elif defined(USE_DISPLAY_LIB_AF_GFX)
        _display(/*w*/ 128, /*h*/ 64, /*twi*/ &Wire, /*rst_pin*/ 16),
#endif
        _yOffset(0),
        _isOn(true),
        _nextTimeOffTS(0)
    { }

    void onStart() override;

    long onLoop() override;

private:
    SdbLock& _ioLock;
    SdbLock& _dataLock;

#if defined(USE_DISPLAY_LIB_U8G2)
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C _u8g2;
#elif defined(USE_DISPLAY_LIB_AF_GFX)
    Adafruit_SSD1306 _display;
#endif
    int _yOffset;
    long _lastDistMM;
    long* _sharedDistMM;
    long _nextTimeOffTS;
    bool _isOn;
    
    void set_next_time_off() {
        _nextTimeOffTS = millis() + DISPLAY_TIME_ON_MS;
    }

    void turn_off();
    void draw();
    void update();
};

#endif // __INC_SDB_MOD_DISPLAY_H
