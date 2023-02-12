## SDB Module Manager.

from sdbdatastore import SdbDataStore
import time

class SdbModuleManager:

    def __init__(self):
        self._mods = []
        self._data_store = SdbDataStore()

    def register(self, mod):
        self._mods.append(mod)

    def start(self):
        for m in self._mods:
            m.onStart()

    def loop_forever(self):
        while True:
            start_ms = time.ticks_ms()
            next_ms = start_ms + 2000 # default: 2sec loop
            for m in self._mods:
                mod_ms = time.ticks_ms()
                # TBD: for "prod" log&ignore loop exception, remove from mod list to retry.
                # For dev, it's better to not trap/ignore exceptions, and let this fail.
                next_sec = m.onLoop()
                mod_ms += next_sec * 1000
                if mod_ms < next_ms:
                    next_ms = mod_ms
            delta_ms = next_ms - start_ms
            if delta_ms > 0:
                time.sleep(delta_ms / 1000)

    def data_store(self):
        return self._data_store

##
