// Test Entry Point

#include <stdio.h>

#define ESP32 1

#include <common.h>

class MainTest {
   public:
    void init() {
        printf("Hello World");
    }

};

int main() {
    MainTest test;
    test.init();
    return 0;
}
