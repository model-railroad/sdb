#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <HardwareSerial.h>

//import something;

#define DEBUG_PRINTF(x)   { Serial.printf x ; }

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    pinMode(BUILTIN_LED, OUTPUT);
    pinMode(19, OUTPUT);
    DEBUG_PRINTF( ("SDB running on Core %d, compiled using C++ %d\n", xPortGetCoreID(), __cplusplus) );
}

void loop() {
    DEBUG_PRINTF( ("loop led pin 19+%d\n", BUILTIN_LED) );
    digitalWrite(BUILTIN_LED, HIGH);
    digitalWrite(19, HIGH);
    delay(250 /*ms*/);
    digitalWrite(BUILTIN_LED, LOW);
    digitalWrite(19, LOW);
    delay(1000 /*ms*/);
}
