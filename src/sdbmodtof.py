## SDB Module: Blinky.

from machine import Pin, SoftI2C
from sdbmod import SdbModule
import sdbmodblinky
import sdbmoddisplay
import ssd1306
import time
import vl53l0x

class SdbModuleToF(SdbModule):

    def __init__(self, manager):
        # - manager: reference to ModuleManager singleton.
        SdbModule.__init__(self, manager, "tf")

    def onStart(self):
        # Returns: nothing.
        sda = Pin(21, Pin.OUT, Pin.PULL_UP)
        scl = Pin(22, Pin.OUT, Pin.PULL_UP)
        i2c = SoftI2C(scl=scl, sda=sda, freq=450000)
        try:
            self._tof = vl53l0x.VL53L0X(i2c)
        except OSError as e:
            print("@@ TOF exception:", e)
            self._tof = None
        self._tmp = int(time.ticks_ms() / 1000 / 5)  # 5 sec
        self._repeat_ms = 50

    # Temporary test to see impact of measuring budget
    def temp(self):
        _tmp = int(time.ticks_ms() / 1000 / 5)  # 5 sec
        if _tmp != self._tmp:
            self._tmp = _tmp
            # bugdet is us fom 20ms up to 200ms
            # try: 0/50/100/150/200 with min 20
            print(_tmp)
            budget_ms = min(200, max(20, 50*(_tmp % 5)))
            self._repeat_ms = budget_ms
            self._tof.set_measurement_timing_budget(budget_ms * 1000)
            print("@@ TOF budget =", self._tof.measurement_timing_budget(), "μs")

    def onLoop(self):
        # Returns: float seconds before next loop.
        while self._msg_queue:
            print("ToF msg: ", self._msg_queue.pop(0))

        if self._tof is not None:
            try:
                dist_mm = self._tof.distance()
                store = self._manager.data_store()
                store.put(sdbmoddisplay.DS_DISP_DIST_MM, dist_mm)
                store.put(sdbmodblinky.DS_LED_TRIGGER, dist_mm < 100) # 300mm=30cm
            except OSError as e:
                print("@@ TOF exception:", e)
                self._tof = None

        if self._tof is not None:
            self.temp()

        # Call about every 50ms
        return self._repeat_ms / 1000

##
