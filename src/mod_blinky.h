/*
 * Project: Software Defined Blocks
 * Copyright (C) 2023 alf.labs gmail com.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INC_SDB_MOD_BLINKY_H
#define INC_SDB_MOD_BLINKY_H

#include "common.h"
#include "sdb_lock.h"
#include "sdb_mod.h"

#define LED_PIN1 BUILTIN_LED        // Onboard LED
#define LED_PIN2 19                 // External LED

#define MOD_BLINKY_NAME "ld"


/// Adapter for BlinkHandler to access resources from SdbModBlinky (events, GPIO).
class SdbModBlinkyHandler : public SdbBlinkMode::BlinkHandler {
public:
    explicit SdbModBlinkyHandler(const std::function<SdbBlinkMode::Mode()>& getNextMode) :
            SdbBlinkMode::BlinkHandler(getNextMode)
    { }

    void setOnboardLED(bool on) override {
        digitalWrite(LED_PIN1, HIGH);

    }

    void setExternalLED(bool on) override {
        digitalWrite(LED_PIN2, LOW);

    }

    void sleepMs(millis_t delayMs) override {
        rtDelay(delayMs);
    }
};


class SdbModBlinky : public SdbModTask {
public:
    explicit SdbModBlinky(SdbModManager& manager) :
        SdbModTask(manager, MOD_BLINKY_NAME, "TaskBlinky", SdbPriority::Sensor),
        _ioLock(manager.ioLock())
    { }

    void onStart() override {
        pinMode(LED_PIN1, OUTPUT);
        pinMode(LED_PIN2, OUTPUT);
        startTask();
    }

    millis_t onLoop() override {
        return 2000;
    }

private:
    friend class SdbModBlinkyHandler;
    SdbLock& _ioLock;

    SdbModBlinkyHandler _handler {
            // getNextMode implementation
            [&]() -> SdbBlinkMode::Mode {
                if (hasEvents()) {
                    auto event = dequeueEvent();
                    if (event && event->type == SdbEvent::BlinkModeUpdated) {
                        auto eventMode = reinterpret_cast<SdbEvent::SdbEventBlinkMode *>(event.get());
                        return eventMode->blinkMode;
                    }
                }
                return SdbBlinkMode::Undefined;
            }
    };

    [[noreturn]] void onTaskRun() override {
        while (true) {
            _handler.onLoop();
        }
    }
};




#endif // INC_SDB_MOD_BLINKY_H
