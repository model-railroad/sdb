#include "mod_display.h"

void SdbModDisplay::onStart() {
    _sharedDistMM = _manager.dataStore().ptrLong(SdbKey::TofDistanceMM, 2000);

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

long SdbModDisplay::onLoop() {
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
            draw();
            update();
        }
    }
    // Refresh more often when we have changes.
    return changes ? 50 : 250;
}

void SdbModDisplay::turn_off() {
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
void SdbModDisplay::draw() {
    _isOn = true;
    int y = abs(_yOffset - 8);
    
#if defined(USE_DISPLAY_LIB_U8G2)
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

void SdbModDisplay::update() {
    SdbMutex io_mutex(_ioLock);
#if defined(USE_DISPLAY_LIB_U8G2)
    _u8g2.sendBuffer();
#elif defined(USE_DISPLAY_LIB_AF_GFX)
    _display.display();
#endif
}
