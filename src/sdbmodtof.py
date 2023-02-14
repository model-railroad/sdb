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
            self._tof.set_measurement_timing_budget(50 * 1000)  # 50 ms
        except OSError as e:
            print("@@ TOF exception:", e)
            self._tof = None

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

        # Call about every 100ms
        return 0.1

##
