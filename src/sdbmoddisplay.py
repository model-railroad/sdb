## SDB Module: Blinky.

from machine import Pin, SoftI2C
from sdbmod import SdbModule
import ssd1306
import time


OLED_SX = const(128)
OLED_SY = const(64)
def _init_oled():
    rst = Pin(16, Pin.OUT)
    rst.value(1)
    sda = Pin(4, Pin.OUT, Pin.PULL_UP)
    scl = Pin(15, Pin.OUT, Pin.PULL_UP)
    i2c = SoftI2C(scl=scl, sda=sda, freq=450000)
    oled = ssd1306.SSD1306_I2C(OLED_SX, OLED_SY, i2c, addr=0x3c)
    return oled

oled_y_offset = 0
oled_msg = "MicroPython"
YTXT = 15
DY = OLED_SY - 35
def _update_oled(oled, ratio, msg = None):
    global oled_y_offset, oled_msg
    if msg: oled_msg = msg
    y = 0 + abs(oled_y_offset - DY//2)
    oled.fill(0)
    oled.text("ESP32", 45, y)
    y += YTXT
    oled.text(oled_msg, 20, y)
    y += YTXT
    sx = OLED_SX-1
    oled.rect(0, y, sx, YTXT, 1)
    oled.fill_rect(0, y, int(sx * ratio), YTXT, 1)
    oled.show()
    oled_y_offset = (oled_y_offset + 1) % DY

# ToF Distance     0123456789 (float)
DS_DISP_DIST_MM = "ld:dist.f"

class SdbModuleDisplay(SdbModule):

    def __init__(self, manager):
        # - manager: reference to ModuleManager singleton.
        SdbModule.__init__(self, manager, "dp")

    def onStart(self):
        # Returns: nothing.
        self._oled = _init_oled()

    def onLoop(self):
        # Returns: float seconds before next loop.
        while self._msg_queue:
            print("Display msg: ", self._msg_queue.pop(0))

        dist_mm = self._manager.data_store().get(DS_DISP_DIST_MM, 0.0)

        _update_oled(self._oled, dist_mm / 1000, "%.2f cm" % (dist_mm / 10))

        # Call about every 250ms
        return 0.250

##
