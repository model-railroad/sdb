#ifndef __INC_SDB_MOD_DISPLAY_H
#define __INC_SDB_MOD_DISPLAY_H

// Important: I2C controller 0 (aka &Wire ) is used here.
//            I2C controller 1 (aka &Wire1) is used in mod_tof.
// WifiKit32 OLED I2C address is 0x3C, pins sda=4 scl=16 reset=16.

#include "common.h"
#include "sdb_lock.h"
#include "sdb_mod.h"
#include <U8g2lib.h>

#define DISPLAY_TIME_ON_MS 15*1000

#define MOD_DISPLAY_NAME "dp"

enum DisplayState {
    DisplaySensor,
    DisplayWifiAP,
};

class SdbModDisplay : public SdbMod {
public:
    SdbModDisplay(SdbModManager& manager) :
        SdbMod(manager, MOD_DISPLAY_NAME),
        _ioLock(_manager.ioLock()),
        _dataLock(_manager.dataStore().lock()),
        _sharedDistMM(NULL),
        _lastDistMM(0),
        _state(DisplaySensor),
        // U8G2 INIT -- OLED U8G2 constructor for ESP32 WIFI_KIT_32 I2C bus on I2C pins 4+15+16
        // _u8g2(U8G2_R0, /*SCL*/ 15, /*SDA*/ 4, /*RESET*/ 16), // for U8G2_SSD1306_128X64_NONAME_F_SW_I2C
        _u8g2(U8G2_R0, /*RESET*/ 16, /*SCL*/ 15, /*SDA*/ 4), // for U8G2_SSD1306_128X64_NONAME_F_HW_I2C
        _yOffset(0),
        _isOn(true),
        _nextTimeOffTS(0)
    { }

    void onStart() override {
        _sharedDistMM = _manager.dataStore().ptrLong(SdbKey::TofDistanceMM, 2000);

        _u8g2.setBusClock(600000);
        _u8g2.begin();
        DEBUG_PRINTF( ("OLED I2C Bus Clock %d, Address 0x%02x\n", _u8g2.getBusClock(), u8g2_GetI2CAddress(&_u8g2)) );
        _u8g2.setFont(u8g2_font_6x10_tf);
        _u8g2.setFontRefHeightExtendedText();
        _u8g2.setDrawColor(1);
        _u8g2.setFontPosTop();
        _u8g2.setFontDirection(0);

        set_next_time_off();
    }

    long onLoop() override {
        bool changes = false;

        auto event = dequeueEvent();
        switch(event) {
            case SdbEvent::DisplaySensor:
                _state = DisplaySensor;
                break;
            case SdbEvent::DisplayWifiAP:
                _state = DisplayWifiAP;
                break;
        }

        switch(_state) {
            case DisplaySensor:
                changes = loopSensor();
                break;
            case DisplayWifiAP:
                changes = loopWifiAP();
                break;
        }

        // Refresh more often when we have changes.
        return changes ? 50 : 250;
    }

private:
    SdbLock& _ioLock;
    SdbLock& _dataLock;
    DisplayState _state;
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C _u8g2;
    int _yOffset;
    long _lastDistMM;
    long* _sharedDistMM;
    long _nextTimeOffTS;
    bool _isOn;

    bool loopWifiAP() {
        const String& ip = _manager.dataStore().getString(SdbKey::SoftAPIP, "unknown");
        drawWifiAP(ip);
        update();
        return false; // no changes
    }

    bool loopSensor() {
        bool changes = false;
        long newDistMM;
        {
            SdbMutex data_mutex(_dataLock);
            newDistMM = *_sharedDistMM;
        }
        if (_lastDistMM != newDistMM) {
            changes = true;
            _isOn = true;
            _lastDistMM = newDistMM;
            set_next_time_off();
        }

        if (_isOn) {
            if (millis() > _nextTimeOffTS) {
                turn_off();
            } else {
                drawSensor();
                update();
            }
        }
        // Refresh more often when we have changes.
        return changes;
    }

    void set_next_time_off() {
        _nextTimeOffTS = millis() + DISPLAY_TIME_ON_MS;
    }

    void turn_off() {
        if (_isOn) {
            _isOn = false;
            _u8g2.clearBuffer();
            _u8g2.sendBuffer();
        }
    }

    #define YTXT 22
    void drawSensor() {
        _isOn = true;
        int y = abs(_yOffset - 8);
        
        _u8g2.clearBuffer();

        // u8g2.setFont(u8g2_font_6x10_tf); //-- from prepare
        // Font is 6x10, coords are x,y
        _u8g2.setFont(u8g2_font_t0_22b_tf);

        _u8g2.drawStr(0, y, "VL53L0X TEST");
        y += YTXT;
        
        String dt = String(_lastDistMM) + " mm";
        _u8g2.drawStr(0, y, dt.c_str());
        y += YTXT;

        // Frame is an empty Box. Box is filled.
        _u8g2.drawFrame(0, y, 128, 8);
        float w = (128.0f / 2000.0f) * _lastDistMM;
        _u8g2.drawBox(0, y, min(128, max(0, (int)w)), 8);
        
        _yOffset = (_yOffset + 1) % 16;
    }

    void drawWifiAP(const String& ip) {
        int y = abs(_yOffset - 8);
        
        _u8g2.clearBuffer();
        _u8g2.setFont(u8g2_font_t0_22b_tf);

        _u8g2.drawStr(0, y, "SDB Wifi AP");
        y += YTXT;
        
        _u8g2.drawStr(0, y, ip.c_str());
        y += YTXT;

        _yOffset = (_yOffset + 1) % 16;
    }

    void update() {
        SdbMutex io_mutex(_ioLock);
        _u8g2.sendBuffer();
    }
};

#endif // __INC_SDB_MOD_DISPLAY_H
