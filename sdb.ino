#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <HardwareSerial.h>

#define DEBUG_PRINTF(x)   { Serial.printf x ; }

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    pinMode(BUILTIN_LED, OUTPUT);
    DEBUG_PRINTF( ("Grade Crossing INO running on Core %d\n", xPortGetCoreID()) );

}

void loop() {
    DEBUG_PRINTF( ("loop led pin %d\n", BUILTIN_LED) );
    digitalWrite(BUILTIN_LED, LOW);
    delay(250 /*ms*/);
    digitalWrite(BUILTIN_LED, HIGH);
    delay(1000 /*ms*/);
}
