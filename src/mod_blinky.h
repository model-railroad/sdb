#ifndef __INC_SDB_MOD_BLINKY_H
#define __INC_SDB_MOD_BLINKY_H

#include "common.h"
#include "sdb_lock.h"
#include "sdb_mod.h"

#define LED_PIN1 BUILTIN_LED
#define LED_PIN2 19

#define MOD_BLINKY_NAME "ld"

class SdbModBlinky : public SdbModTask {
public:
    SdbModBlinky(SdbModManager& manager) :
        SdbModTask(manager, MOD_BLINKY_NAME, "TaskBlinky", SdbPriority::Sensor),
        _ioLock(manager.ioLock())
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
    SdbLock& _ioLock;

    void onTaskRun() override {
        while (true) {
            {
                SdbMutex io_mutex(_ioLock);
                digitalWrite(LED_PIN1, HIGH);
                digitalWrite(LED_PIN2, LOW);
            }
            rtDelay(250 /*ms*/);

            {
                SdbMutex io_mutex(_ioLock);
                digitalWrite(LED_PIN1, LOW);
                digitalWrite(LED_PIN2, HIGH);
            }
            rtDelay(250 /*ms*/);

            {
                SdbMutex io_mutex(_ioLock);
                digitalWrite(LED_PIN1, LOW);
                digitalWrite(LED_PIN2, LOW);
            }
            rtDelay(1000 /*ms*/);
        }
    }
};


#endif // __INC_SDB_MOD_BLINKY_H
