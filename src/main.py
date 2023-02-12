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

def timeit(label, fn, iter=100, collect=False):
    gc.collect() # garbage collects first
    ms = time.ticks_ms()
    mem = gc.mem_alloc()
    ret = []
    i = 0
    if collect:
        ret = []
        while i < iter:
            ret.append(fn())
            i += 1
    else:
        ret = None
        while i < iter:
            ret = fn()
            i += 1
    ms = (time.ticks_ms() - ms) / iter
    mem = gc.mem_alloc() - mem
    if collect or ret is None:
        print(label, ms, "ms", mem, "bytes")
    else:
        print(label, ret, "->", ms, "ms", mem, "bytes")


def memory_test():
    timeit("Mem used:", gc.mem_alloc)
    timeit("Mem free:", gc.mem_free)
    timeit("Tick ms :", time.ticks_ms)

    kj = 100
    def measure_array2():
        j = 0
        a = [1, 2]
        b = 0
        while j < kj:
            b = a[0]
            a = [ a, j ]
            j += 1
        return a

    def measure_array4():
        j = 0
        a = [1, 2, 3, 4]
        b = 0
        while j < kj:
            b = a[0]
            a = [ a, a[2], a[3], j ]
            j += 1
        return a

    def measure_array5():
        j = 0
        a = [1, 2, 3, 4, 5]
        b = 0
        while j < kj:
            b = a[0]
            a = [ a, a[2], a[3], j , j+1]
            j += 1
        return a

    def measure_tuple():
        j = 0
        a = (1, 2)
        b = 0
        while j < kj:
            b = a[0]
            a = ( a, j )
            j += 1
        return a

    def measure_dict():
        j = 0
        a = { "a": 1, "b": 2, "c": 3, "d": 4 }
        b = 0
        while j < kj:
            b = a["a"]
            a = { "a": a, "b": a["c"], "c": a["d"], "d": j }
            j += 1
        return a

    def measure_dict_long_str():
        # Strings are not interned if > 10 chars.
        # 0123456789|123
        # long keyword X
        j = 0
        a = { "long keyword Xa": 0, "long keyword Xb": 1, "long keyword Xc": 2, "long keyword Xd": 3 }
        b = 0
        while j < kj:
            b = a["long keyword Xa"]
            a = { "long keyword Xa": a, "long keyword Xb": a["long keyword Xc"], "long keyword Xc": a["long keyword Xd"], "long keyword Xd": j }
            j += 1
        return a

    n = 10
    timeit("Arrays 2:", measure_array2, iter=n, collect=True)
    timeit("Arrays 4:", measure_array4, iter=n, collect=True)
    timeit("Arrays 5:", measure_array5, iter=n, collect=True)
    timeit("Tuples  :", measure_tuple, iter=n, collect=True)
    timeit("Dicts   :", measure_dict, iter=n, collect=True)
    timeit("Dicts 2 :", measure_dict_long_str, iter=n, collect=True)

    def dict_string_n(sn):
        sn -= 1
        d = {}
        j = 0
        while j < kj:
            k = f"a%{sn}d" % j
            d[k] = j
            j += 1
        # access the entire dict content
        j = 0
        while j < kj:
            k = f"a%{sn}d" % j
            b = j
            p = 0
            while p < kj:
                b += d[k]
                p += 1
            j += 1
        return d

    n = 5
    for ls in range(5,31,5):
        timeit(f"Dict Str {ls}:", lambda: dict_string_n(ls), iter=n, collect=True)

    def dict_access_intern():
        d = {}
        j = 0
        b = 0
        while j < kj:
            d["key123"] = j
            p = 0
            while p < kj:
                b += d["key123"]
                p += 1
            j += 1

    def dict_access_var(key):
        d = {}
        j = 0
        b = 0
        while j < kj:
            d[key] = j
            p = 0
            while p < kj:
                b += d[key]
                p += 1
            j += 1

    v = 124
    timeit(f"Dict Access Intern:", lambda: dict_access_intern(), iter=n)
    timeit(f"Dict Access Var:", lambda: dict_access_var("key%d" % v), iter=n)


def intern(s):
    exec(f"def _tmp_foo({s}=1): print({s})")
    return s

def string_test():

    str_a = intern("a123456789")
    str_b = intern("a123456789")
    str_c = "a123456789"
    str_d = "a123456789"
    ks = 10000
    def string_is_intern():
        s = 0
        while s < ks:
            str_a is str_b
            s += 1

    def string_is_regular():
        s = 0
        while s < ks:
            str_c is str_d
            s += 1

    def string_eq_intern():
        s = 0
        while s < ks:
            str_a == str_b
            s += 1

    def string_eq_regular():
        s = 0
        while s < ks:
            str_c == str_d
            s += 1

    timeit("string_is_intern:", string_is_intern)
    timeit("string_is_regular:", string_is_regular)
    timeit("string_eq_intern:", string_eq_intern)
    timeit("string_eq_regular:", string_eq_regular)

    timeit("Print ln:", lambda: print("time ", time.ticks_ms()), iter=10)


if __name__ == "__main__":
    print("CPU freq:", machine.freq() / 1000 / 1000, "MHz")
    print("Flash:", esp.flash_size() / 1024 / 1024, "MB")
    print("Start:", esp.flash_user_start() / 1024 / 1024, "MB")
    m = os.statvfs("//")
    print("VFS  :", (m[0]*m[3]) / 1024 / 1024, "MB free out of", (m[0]*m[2]) / 1024 / 1024, "MB")

    memory_test()
    string_test()
    timeit("Mem used:", gc.mem_alloc)
    timeit("Mem free:", gc.mem_free)

    main_led_loop()
    # main_loop()
