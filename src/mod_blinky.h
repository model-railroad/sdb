#ifndef __INC_SDB_MOD_BLINKY_H
#define __INC_SDB_MOD_BLINKY_H

#include "common.h"
#include "sdb_mod.h"

#define LED_PIN1 BUILTIN_LED
#define LED_PIN2 19

class SdbModBlinky : public SdbMod {
public:
    SdbModBlinky(SdbModManager& manager) :
        SdbMod(manager, "ld"),
        _state(State::Init) {
    }

    void onStart() override {
        pinMode(LED_PIN1, OUTPUT);
        pinMode(LED_PIN2, OUTPUT);
    }

    long onLoop() override {
        long wait_ms = 1000;
        if (_state == State::Init) {
            wait_ms = _manager.schedule(250, [this]() { state1_1On2Off(); });
        }
        return wait_ms;
    }

private:
    enum class State {
        Init,
        State1_1On2Off,
        State2_1Off2On,
        State3_1Off2Off_Pause,
    };
    State _state;

    void state1_1On2Off() {
        _state = State::State1_1On2Off;
        digitalWrite(LED_PIN1, HIGH);
        digitalWrite(LED_PIN2, LOW);
        _manager.schedule(250, [this]() { state2_1Off2On(); });
    }

    void state2_1Off2On() {
        _state = State::State2_1Off2On;
        digitalWrite(LED_PIN1, LOW);
        digitalWrite(LED_PIN2, HIGH);
        _manager.schedule(250, [this]() { state3_1Off2Off_Pause(); });
    }

    void state3_1Off2Off_Pause() {
        _state = State::State3_1Off2Off_Pause;
        digitalWrite(LED_PIN1, LOW);
        digitalWrite(LED_PIN2, LOW);
        _manager.schedule(1000, [this]() { state1_1On2Off(); });
    }
};

#endif // __INC_SDB_MOD_BLINKY_H
