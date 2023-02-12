import esp
import gc
import machine
import micropython
import os
import sdbmodblinky
import sdbmoddisplay
import sdbmodtof
from sdbmodmanager import SdbModuleManager
import _thread


def main_loop():
    print("SDB ModuleManager: Create")
    global sdb_mod_manager
    sdb_mod_manager = SdbModuleManager()
    sdbmodblinky.SdbModuleBlinky(sdb_mod_manager)
    sdbmoddisplay.SdbModuleDisplay(sdb_mod_manager)
    sdbmodtof.SdbModuleToF(sdb_mod_manager)

    print("SDB ModuleManager: Start")
    sdb_mod_manager.start()

    print("SDB ModuleManager: Loop")
    sdb_mod_manager.loop_forever()
    print("End main loop")

if __name__ == "__main__":
    print("CPU freq:", machine.freq() / 1000 / 1000, "MHz")
    print("Flash:", esp.flash_size() / 1024 / 1024, "MB")
    m = os.statvfs("//")
    print("VFS  :", (m[0]*m[3]) / 1024 / 1024, "MB free out of", (m[0]*m[2]) / 1024 / 1024, "MB")
    print("Mem used:", gc.mem_alloc(), "Bytes")
    print("Mem free:", gc.mem_free(), "Bytes")
    micropython.mem_info()
    # 
    _thread.start_new_thread(main_loop, ())
    #

##
