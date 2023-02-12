## SDB Module: Blinky.

from machine import Pin
from sdbmod import SdbModule
import time

# LED 2 trigger   0123456789 (boolean)
DS_LED_TRIGGER = "ld:trigr.b"

class SdbModuleBlinky(SdbModule):

    def __init__(self, manager):
        # - manager: reference to ModuleManager singleton.
        SdbModule.__init__(self, manager, "ld")

    def onStart(self):
        # Returns: nothing.
        self._led_pin1 = Pin(25, Pin.OUT)
        self._led_pin2 = Pin(19, Pin.OUT)
        self._led_pin1.off()
        self._led_pin2.off()
        self._state1 = False
        self._state2 = False
        self._next_on = time.ticks_ms() + 1000
        self._next_off = self._next_on + 250

    def blink(self):
        now_ms = time.ticks_ms()
        if not self._state1 and now_ms >= self._next_on:
            # State off->on
            self._led_pin1.on()
            self._state1 = True
        elif self._state1 and now_ms >= self._next_off:
            self._led_pin1.off()
            self._state1 = False
            self._next_on = now_ms + 1000
            self._next_off = self._next_on + 250

    def onLoop(self):
        # Returns: float seconds before next loop.
        while self._msg_queue:
            print("Blinky msg: ", self._msg_queue.pop(0))

        self.blink()

        s = self._manager.data_store().get(DS_LED_TRIGGER, False)
        if s != self._state2:
            self._state2 = s == True
            self._led_pin2.value(1 if self._state2 else 0)

        # Call about every 250ms
        return 0.250

##
