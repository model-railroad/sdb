#ifndef __INC_SDB_MOD_DISPLAY_H
#define __INC_SDB_MOD_DISPLAY_H

// Important: I2C controller 0 (aka &Wire ) is used here.
//            I2C controller 1 (aka &Wire1) is used in mod_tof.
// WifiKit32 OLED I2C address is 0x3C, pins sda=4 scl=16 reset=16.

#include "common.h"
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


class SdbModDisplay : public SdbMod {
public:
    SdbModDisplay(SdbModManager& manager) :
        SdbMod(manager, "dp"),
        _imported_dist_mm(NULL),
        _last_dist_mm(0),
#if defined(USE_DISPLAY_LIB_U8G2)
        // U8G2 INIT -- OLED U8G2 constructor for ESP32 WIFI_KIT_32 I2C bus on I2C pins 4+15+16
        // _u8g2(U8G2_R0, /*SCL*/ 15, /*SDA*/ 4, /*RESET*/ 16), // for U8G2_SSD1306_128X64_NONAME_F_SW_I2C
        _u8g2(U8G2_R0, /*RESET*/ 16, /*SCL*/ 15, /*SDA*/ 4), // for U8G2_SSD1306_128X64_NONAME_F_HW_I2C
#elif defined(USE_DISPLAY_LIB_AF_GFX)
        _display(/*w*/ 128, /*h*/ 64, /*twi*/ &Wire, /*rst_pin*/ 16),
#endif
        _y_offset(0),
        _is_on(true),
        _next_time_off_ts(0)
    { }

    void onStart() override {
        _imported_dist_mm = _manager.dataStore().ptrLong(SdbKey::TofDistanceMM, 2000);

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

        set_next_time_off();
    }

    long onLoop() override {
        long new_dist_mm = *_imported_dist_mm;
        if (_last_dist_mm != new_dist_mm) {
            _is_on = true;
            _last_dist_mm = new_dist_mm;
            set_next_time_off();
        }

        if (_is_on) {
            if (millis() > _next_time_off_ts) {
                turn_off();
            } else {
                draw();
            }
        }
        return 250;
    }

private:
#if defined(USE_DISPLAY_LIB_U8G2)
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C _u8g2;
#elif defined(USE_DISPLAY_LIB_AF_GFX)
    Adafruit_SSD1306 _display;
#endif
    int _y_offset;
    long _last_dist_mm;
    long* _imported_dist_mm;
    long _next_time_off_ts;
    bool _is_on;
    
    #define DISPLAY_TIME_ON_MS 30*1000
    void set_next_time_off() {
        _next_time_off_ts = millis() + DISPLAY_TIME_ON_MS;
    }

    void turn_off() {
        if (_is_on) {
            _is_on = false;
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
    void draw() {
        _is_on = true;
        int y = abs(_y_offset - 8);
        
#if defined(USE_DISPLAY_LIB_U8G2)
        _u8g2.clearBuffer();

        // u8g2.setFont(u8g2_font_6x10_tf); //-- from prepare
        // Font is 6x10, coords are x,y
        _u8g2.setFont(u8g2_font_t0_22b_tf);

        _u8g2.drawStr(0, y, "VL53L0X TEST");
        y += YTXT;
        
        String dt = String(_last_dist_mm) + " mm";
        _u8g2.drawStr(0, y, dt.c_str());
        y += YTXT;

        // Frame is an empty Box. Box is filled.
        _u8g2.drawFrame(0, y, 128, 8);
        float w = (128.0f / 2000.0f) * _last_dist_mm;
        _u8g2.drawBox(0, y, min(128, max(0, (int)w)), 8);

        _u8g2.sendBuffer();
#elif defined(USE_DISPLAY_LIB_AF_GFX)
        _display.clearDisplay();

        _display.setCursor(0,y);
        _display.print("VL53L0X");
        y += YTXT;

        String dt = String(_last_dist_mm) + " mm";
        _display.setCursor(0,y);
        _display.print(dt.c_str());
        y += YTXT;

        // Frame is an empty Box. Box is filled.
        _display.drawRect(0, y, 128, 8, WHITE);
        float w = (128.0f / 2000.0f) * _last_dist_mm;
        _display.fillRect(0, y, min(128, max(0, (int)w)), 8, WHITE);

        _display.display();
#endif
        
        _y_offset = (_y_offset + 1) % 16;
    }
};

#endif // __INC_SDB_MOD_DISPLAY_H
