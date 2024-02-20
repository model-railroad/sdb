#pragma once

#include <stdint.h>

// -- ESP IDF stuff

typedef int esp_err_t;

#define ESP_OK 0
#define ESP_FAIL -1

const char * esp_err_to_name(esp_err_t err) { return "error"; }

#define configTICK_RATE_HZ 1000
#define portTICK_PERIOD_MS ( ( TickType_t ) 1000 / configTICK_RATE_HZ )

void vTaskDelay(unsigned long ticks) { }

// -- Arduino stuff

#define BUILTIN_LED 25

unsigned long millis() { return 42UL; }
void delay(unsigned long millis) { /* no-op */ }

#define LOW               0x0
#define HIGH              0x1

#define INPUT             0x01
#define OUTPUT            0x03
#define PULLUP            0x04
#define INPUT_PULLUP      0x05
#define PULLDOWN          0x08
#define INPUT_PULLDOWN    0x09
#define OPEN_DRAIN        0x10
#define OUTPUT_OPEN_DRAIN 0x12
#define ANALOG            0xC0

void pinMode(uint8_t pin, uint8_t mode) { }
void digitalWrite(uint8_t pin, uint8_t val) { }
int digitalRead(uint8_t pin) { return 42; }
