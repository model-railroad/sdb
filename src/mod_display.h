#ifndef __INC_SDB_MOD_DISPLAY_H
#define __INC_SDB_MOD_DISPLAY_H

#include "common.h"
#include "sdb_mod.h"
#include <U8g2lib.h>


class SdbModDisplay : public SdbMod {
public:
    SdbModDisplay(SdbModManager& manager) :
        SdbMod(manager, "dp"),
        // U8G2 INIT -- OLED U8G2 constructor for ESP32 WIFI_KIT_32 I2C bus on I2C pins 4+15+16
        _u8g2(U8G2_R0, /*SCL*/ 15, /*SDA*/ 4, /*RESET*/ 16), // for U8G2_SSD1306_128X64_NONAME_F_SW_I2C
        _y_offset(0),
        _is_on(true),
        _next_time_off_ts(0)
    { }

    void onStart() override {
        _u8g2.setBusClock(600000);
        _u8g2.begin();
        DEBUG_PRINTF( ("OLED I2C Bus Clock %d\n", _u8g2.getBusClock()) );
        _u8g2.setFont(u8g2_font_6x10_tf);
        _u8g2.setFontRefHeightExtendedText();
        _u8g2.setDrawColor(1);
        _u8g2.setFontPosTop();
        _u8g2.setFontDirection(0);

        set_next_time_off();
    }

    long onLoop() override {
        long new_dist_mm = _manager.dataStore().get(SdbKey::TofDistanceMM, 2000);
        if (_tof_dist_mm != new_dist_mm) {
            _is_on = true;
            _tof_dist_mm = new_dist_mm;
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
    U8G2_SSD1306_128X64_NONAME_F_SW_I2C _u8g2;
    int _y_offset;
    long _tof_dist_mm;
    long _next_time_off_ts;
    bool _is_on;
    
    #define DISPLAY_TIME_ON_MS 10*1000
    void set_next_time_off() {
        _next_time_off_ts = millis() + DISPLAY_TIME_ON_MS;
    }

    void turn_off() {
        if (_is_on) {
            _is_on = false;
            _u8g2.clearBuffer();
            _u8g2.sendBuffer();
        }
    }

    #define YTXT 22
    void draw() {
        _is_on = true;
        _u8g2.clearBuffer();

        int str_len;
        int y = abs(_y_offset - 8);
        
        // u8g2.setFont(u8g2_font_6x10_tf); //-- from prepare
        // Font is 6x10, coords are x,y
        _u8g2.setFont(u8g2_font_t0_22b_tf);

        _u8g2.drawStr(0, y, "VL53L0X TEST");
        y += YTXT;
        
        String dt = String(_tof_dist_mm) + " mm";
        _u8g2.drawStr(0, y, dt.c_str());
        y += YTXT;

        // Frame is an empty Box. Box is filled.
        _u8g2.drawFrame(0, y, 128, 8);
        float w = (128.0f / 2000.0f) * _tof_dist_mm;
        _u8g2.drawBox(0, y, min(128, max(0, (int)w)) , 8);
        
        _y_offset = (_y_offset + 1) % 16;

        _u8g2.sendBuffer();
    }
};

#endif // __INC_SDB_MOD_DISPLAY_H
