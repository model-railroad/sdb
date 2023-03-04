#ifndef __INC_SDB_MOD_BLINKY_H
#define __INC_SDB_MOD_BLINKY_H

#include "common.h"
#include "sdb_lock.h"
#include "sdb_mod.h"

#define LED_PIN1 BUILTIN_LED
#define LED_PIN2 19


class SdbModBlinky : public SdbModTask {
public:
    SdbModBlinky(SdbModManager& manager) :
        SdbModTask(manager, "ld", "TaskBlinky", SdbPriority::Sensor),
        _io_lock(manager.ioLock())
    { }

    void onStart() override {
        pinMode(LED_PIN1, OUTPUT);
        pinMode(LED_PIN2, OUTPUT);
        startTask();
    }

    long onLoop() override {
        return 2000;
    }

private:
    SdbLock& _io_lock;

    void onTaskRun() override {
        while (true) {
            {
                SdbMutex io_mutex(_io_lock);
                digitalWrite(LED_PIN1, HIGH);
                digitalWrite(LED_PIN2, LOW);
            }
            rtDelay(250 /*ms*/);

            {
                SdbMutex io_mutex(_io_lock);
                digitalWrite(LED_PIN1, LOW);
                digitalWrite(LED_PIN2, HIGH);
            }
            rtDelay(250 /*ms*/);

            {
                SdbMutex io_mutex(_io_lock);
                digitalWrite(LED_PIN1, LOW);
                digitalWrite(LED_PIN2, LOW);
            }
            rtDelay(1000 /*ms*/);
        }
    }
};


#endif // __INC_SDB_MOD_BLINKY_H
