#include "mod_blinky.h"

#define LED_PIN1 BUILTIN_LED
#define LED_PIN2 19

void SdbModBlinky::onStart() {
    pinMode(LED_PIN1, OUTPUT);
    pinMode(LED_PIN2, OUTPUT);
    startTask();
}

long SdbModBlinky::onLoop() {
    return 2000;
}

void SdbModBlinky::onTaskRun() {
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
