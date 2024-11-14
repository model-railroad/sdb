/*
 * Project: Software Defined Blocks
 * Copyright (C) 2023 alf.labs gmail com.
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

module;

// Important: I2C controller 0 (aka &Wire ) is used here.
//            I2C controller 1 (aka &Wire1) is used in mod_tof.
// WifiKit32 OLED I2C address is 0x3C, pins sda=4 scl=16 reset=16.

#include "common.h"
#include "mod_display.h"

#if defined(USE_DISPLAY_LIB_U8G2)
#include <U8g2lib.h>
#endif

//---------------
export module SDB.Mod.Display;

#ifdef MOD_DISPLAY_ENABLED

import SDB.Lock;
import SDB.Mod;
import SDB.ModManager;
import SDB.Sensor;
import SDB.ToF;

#define DISPLAY_TIME_SENSOR_ON_MS (15*1000)
#define DISPLAY_TIME_WIFI_ON_MS   (2*1000)

#define MOD_DISPLAY_NAME "dp"

//---------------
enum DisplayState {
    DisplaySensor,
    DisplayWifiAP,
    DisplayWifiSTA,
};


export class SdbModDisplay : public SdbMod {
public:
    explicit SdbModDisplay(SdbModManager& manager) :
        SdbMod(manager, MOD_DISPLAY_NAME),
        _ioLock(_manager.ioLock()),
        _dataLock(_manager.dataStore().lock()),
        _apMode(false),
        _lastRefresh(0),
        _state(DisplaySensor),
#if defined(USE_DISPLAY_LIB_U8G2)
        // U8G2 INIT -- OLED U8G2 constructor for ESP32 WIFI_KIT_32 I2C bus on I2C pins 4+15+16
        // _u8g2(U8G2_R0, /*SCL*/ 15, /*SDA*/ 4, /*RESET*/ 16), // for U8G2_SSD1306_128X64_NONAME_F_SW_I2C
        _u8g2(U8G2_R0, /*RESET*/ 16, /*SCL*/ 15, /*SDA*/ 4), // for U8G2_SSD1306_128X64_NONAME_F_HW_I2C
#endif
        _yOffset(0),
        _isOn(true),
        _nextTimeOffTS(0)
    { }

    void onStart() override {
#if defined(USE_DISPLAY_LIB_U8G2)
        _u8g2.setBusClock(600000);
        _u8g2.begin();
        DEBUG_PRINTF( ("OLED I2C Bus Clock %d, Address 0x%02x\n", _u8g2.getBusClock(), u8g2_GetI2CAddress(&_u8g2)) );
        _u8g2.setFont(u8g2_font_6x10_tf);
        _u8g2.setFontRefHeightExtendedText();
        _u8g2.setDrawColor(1);
        _u8g2.setFontPosTop();
        _u8g2.setFontDirection(0);
#endif

        setNextTimeOff();
    }

    millis_t onLoop() override {
        bool changes = false;

        auto event = dequeueEvent();
        if (event) {
            switch (event->type) {
                case SdbEvent::DisplaySensor:
                    _state = DisplaySensor;
                    break;
                case SdbEvent::DisplayWifiAP:
                    _state = DisplayWifiAP;
                    // Display the wifi info for a few seconds, then back to sensor state.
                    _manager.schedule(DISPLAY_TIME_WIFI_ON_MS, [this](void) {
                        _manager.queueEvent(MOD_DISPLAY_NAME, SdbEvent::DisplaySensor);
                    });
                    break;
                case SdbEvent::DisplayWifiSTA:
                    _state = DisplayWifiSTA;
                    // Display the wifi info for a few seconds, then back to sensor state.
                    _manager.schedule(DISPLAY_TIME_WIFI_ON_MS, [this](void) {
                        _manager.queueEvent(MOD_DISPLAY_NAME, SdbEvent::DisplaySensor);
                    });
                    break;
            }
        }

        switch(_state) {
            case DisplaySensor:
                changes = loopSensor();
                break;
            case DisplayWifiAP:
                changes = loopWifiAP();
                break;
            case DisplayWifiSTA:
                changes = loopWifiSTA();
                break;
        }

        // Refresh more often when we have changes.
        return changes ? 50 : 250;
    }

private:
    bool _apMode;
    SdbLock& _ioLock;
    SdbLock& _dataLock;
    DisplayState _state;
    int _yOffset;
    long _lastRefresh;
    millis_t _nextTimeOffTS;
    bool _isOn;
    #if defined(USE_DISPLAY_LIB_U8G2)
        U8G2_SSD1306_128X64_NONAME_F_HW_I2C _u8g2;
    #endif

    bool loopWifiAP() {
        _apMode = true;
        drawWifiAP();
        update();
        return false; // no changes
    }

    bool loopWifiSTA() {
        _apMode = false;
        drawWifiSTA();
        update();
        return false; // no changes
    }

    bool loopSensor() {
        long _newRefresh = 0;
#ifdef MOD_TOF_ENABLED
        for(auto sref: _manager.sensors()) {
            auto& tp = reinterpret_cast<SdbSensorTof&>(sref.get());
            long newDistMM = tp.lastDistMM();
            _newRefresh = _newRefresh * 1000 + newDistMM;
        }
#endif
        bool changes = _lastRefresh != _newRefresh;

        if (changes) {
            _isOn = true;
            _lastRefresh = _newRefresh;
            setNextTimeOff();
        }

        if (_isOn) {
            // In sensor mode, turn off display after some inactivity.
            // But before that, display the wifi state for 2 seconds.
            millis_t now = millis();
            if (now > _nextTimeOffTS) {
                turnOff();
            } else if (now > _nextTimeOffTS - DISPLAY_TIME_WIFI_ON_MS) {
                if (_apMode) {
                    drawWifiAP();
                } else {
                    drawWifiSTA();
                }
                update();
            } else {
                drawSensor();
                update();
            }
        }
        // Refresh more often when we have changes.
        return changes;
    }

    void setNextTimeOff() {
        _nextTimeOffTS = millis() + DISPLAY_TIME_SENSOR_ON_MS;
    }

    void turnOff() {
        if (_isOn) {
            _isOn = false;
#if defined(USE_DISPLAY_LIB_U8G2)
            _u8g2.clearBuffer();
            _u8g2.sendBuffer();
#endif
        }
    }

    #define YTXT 22
    void drawSensor() {
        _isOn = true;
        int y = abs(_yOffset - 8);

#if defined(USE_DISPLAY_LIB_U8G2)
        _u8g2.clearBuffer();

        // u8g2.setFont(u8g2_font_6x10_tf); //-- from prepare
        // Font is 6x10, coords are x,y
        _u8g2.setFont(u8g2_font_t0_22b_tf);

        _u8g2.drawStr(0, y, "SDB: VL53L0X");
        y += YTXT;

        for(auto& sref: _manager.sensors()) {
            sref.get().draw(_u8g2, y);
            y += YTXT;
        }
#endif
        
        _yOffset = (_yOffset + 1) % 16;
    }

    void drawWifiAP() {
         const String* ip = _manager.dataStore().getString(SdbKey::WifiApIpStr);
         if (ip != nullptr) {
            drawWifiIP(*ip);
         }
    }

    void drawWifiSTA() {
         const String* ip = _manager.dataStore().getString(SdbKey::WifiStaIpStr);
         if (ip != nullptr) {
            drawWifiIP(*ip);
         }
    }

    void drawWifiIP(const String& ip) {
#if defined(USE_DISPLAY_LIB_U8G2)
        int y = abs(_yOffset - 8);

        _u8g2.clearBuffer();
        _u8g2.setFont(u8g2_font_t0_22b_tf);

        _u8g2.drawStr(0, y, "SDB Wifi: ");
        y += YTXT;
        _u8g2.drawStr(0, y, ip.c_str());
        y += YTXT;
        _yOffset = (_yOffset + 1) % 16;
#endif
    }

    void update() {
        SdbMutex io_mutex(_ioLock);
#if defined(USE_DISPLAY_LIB_U8G2)
        _u8g2.sendBuffer();
#endif
    }
};

#endif // MOD_DISPLAY_ENABLED
