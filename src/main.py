import esp
import gc
import machine
from machine import Pin, SoftI2C
import os
import ssd1306
import time
import _thread
import vl53l0x

led_pin1 = Pin(25, Pin.OUT)
led_pin2 = Pin(19, Pin.OUT)

led_pin1.off()
led_pin2.off()


def blink(pin):
    pin.on()
    time.sleep_ms(250)
    pin.off()

def thread_led(pin):
    print("Thread LED", pin)
    while True:
        blink(pin)
        time.sleep_ms(1000)

OLED_SX = const(128)
OLED_SY = const(64)
def init_oled():
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
def update_oled(oled, ratio, msg = None):
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

oled_distance = 0.0
def thread_oled():
    oled = init_oled()
    n = 0
    while True:
        update_oled(oled, oled_distance / 1000, "%.2f mm" % oled_distance)
        n += 1
        time.sleep_ms(50)
        # print("loop",n)

def init_tof():
    sda = Pin(21, Pin.OUT, Pin.PULL_UP)
    scl = Pin(22, Pin.OUT, Pin.PULL_UP)
    i2c = SoftI2C(scl=scl, sda=sda, freq=450000)
    tof = vl53l0x.VL53L0X(i2c)
    return tof

def thread_tof():
    global oled_distance
    tof = init_tof()
    while True:
        oled_distance = tof.distance()
        time.sleep_ms(50)

def main_led_loop():
    _thread.start_new_thread(thread_led, (led_pin1, ))
    time.sleep_ms(500)
    _thread.start_new_thread(thread_led, (led_pin2, ))

def main_loop():
    main_led_loop()
    _thread.start_new_thread(thread_oled, ())
    _thread.start_new_thread(thread_tof, ())
    print("End main loop")

if __name__ == "__main__":
    print("CPU freq:", machine.freq() / 1000 / 1000, "MHz")
    print("Flash:", esp.flash_size() / 1024 / 1024, "MB")
    print("Start:", esp.flash_user_start() / 1024 / 1024, "MB")
    m = os.statvfs("//")
    print("VFS  :", (m[0]*m[3]) / 1024 / 1024, "MB free out of", (m[0]*m[2]) / 1024 / 1024, "MB")
    print("Mem used:", gc.mem_alloc() / 1024 / 1024, "MB")
    print("Mem free:", gc.mem_free() / 1024 / 1024, "MB")
    main_loop()
