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

#ifndef INC_SDB_MOD_DISPLAY_H
#define INC_SDB_MOD_DISPLAY_H

// Important: I2C controller 0 (aka &Wire ) is used here.
//            I2C controller 1 (aka &Wire1) is used in mod_tof.
// WifiKit32 OLED I2C address is 0x3C, pins sda=4 scl=16 reset=16.

#include "common.h"
#include "sdb_lock.h"
#include "sdb_mod.h"
#include "mod_tof.h"

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

#define DISPLAY_TIME_SENSOR_ON_MS (15*1000)
#define DISPLAY_TIME_WIFI_ON_MS   (2*1000)

#define MOD_DISPLAY_NAME "dp"

enum DisplayState {
    DisplaySensor,
    DisplayWifiAP,
    DisplayWifiSTA,
};

class SdbModDisplay : public SdbMod {
public:
    explicit SdbModDisplay(SdbModManager& manager) :
        SdbMod(manager, MOD_DISPLAY_NAME),
        _ioLock(_manager.ioLock()),
        _dataLock(_manager.dataStore().lock()),
        _apMode(false),
        _state(DisplaySensor),
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
#elif defined(USE_DISPLAY_LIB_AF_GFX)
        Wire.begin(/*SDA*/ 4, /*SLC*/ 15);
        _display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
        // init done
        _display.display();
        
        _display.setTextSize(2);
        _display.setTextColor(WHITE);
#endif

        setNextTimeOff();
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
        bool changes = false;
        for (int n = 0; n < TOF_NUM; n++) {
            long newDistMM = _manager.dataStore().getLong(ToFSdbKey[n], OUT_OF_RANGE_MM);
            if (_lastDistMM[n] != newDistMM) {
                changes = true;
                _lastDistMM[n] = newDistMM;
            }
        }
        if (changes) {
            _isOn = true;
            setNextTimeOff();
        }

        if (_isOn) {
            // In sensor mode, turn off display after some inactivity.
            // But before that, display the wifi state for 2 seconds.
            long now = millis();
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

#if defined(USE_DISPLAY_LIB_U8G2)
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C _u8g2;
#elif defined(USE_DISPLAY_LIB_AF_GFX)
    Adafruit_SSD1306 _display;
#endif
    int _yOffset;
    long _lastDistMM[TOF_NUM]{};
    long _nextTimeOffTS;
    bool _isOn;
    
    void setNextTimeOff() {
        _nextTimeOffTS = millis() + DISPLAY_TIME_SENSOR_ON_MS;
    }

    void turnOff() {
        if (_isOn) {
            _isOn = false;
#if defined(USE_DISPLAY_LIB_U8G2)
            _u8g2.clearBuffer();
            _u8g2.sendBuffer();
#elif defined(USE_DISPLAY_LIB_AF_GFX)
            _display.clearDisplay();
            _display.display();
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
        
        String dt = String(_lastDistMM[0]);
        if (TOF_NUM > 1) {
            dt += "/";
            dt += String(_lastDistMM[1]);
        }
        dt += " mm";
        _u8g2.drawStr(0, y, dt.c_str());
        y += YTXT;

        // Frame is an empty Box. Box is filled.
        for (int n = 0; n < TOF_NUM; n++) {
            _u8g2.drawFrame(0, y, 128, 8);
            float w = 128.0f / 2000.0f * _lastDistMM[n];
            _u8g2.drawBox(0, y, min(128, max(0, (int)w)), 8);
            y += 10;
        }
#elif defined(USE_DISPLAY_LIB_AF_GFX)
        _display.clearDisplay();

        _display.setCursor(0,y);
        _display.print("VL53L0X");
        y += YTXT;

        String dt = String(_lastDistMM) + " mm";
        _display.setCursor(0,y);
        _display.print(dt.c_str());
        y += YTXT;

        // Frame is an empty Box. Box is filled.
        _display.drawRect(0, y, 128, 8, WHITE);
        float w = (128.0f / 2000.0f) * _lastDistMM;
        _display.fillRect(0, y, min(128, max(0, (int)w)), 8, WHITE);
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
        int y = abs(_yOffset - 8);
        
        _u8g2.clearBuffer();
        _u8g2.setFont(u8g2_font_t0_22b_tf);

        _u8g2.drawStr(0, y, "SDB Wifi: ");
        y += YTXT;
        _u8g2.drawStr(0, y, ip.c_str());
        y += YTXT;

        _yOffset = (_yOffset + 1) % 16;
    }

    void update() {
        SdbMutex io_mutex(_ioLock);
#if defined(USE_DISPLAY_LIB_U8G2)
        _u8g2.sendBuffer();
#elif defined(USE_DISPLAY_LIB_AF_GFX)
        _display.display();
#endif
    }
};

#endif // INC_SDB_MOD_DISPLAY_H
