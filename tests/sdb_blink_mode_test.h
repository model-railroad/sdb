/*
 * Project: Software Defined Blocks
 * Copyright (C) 2024 alf.labs gmail com.
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

// Test SdbLock and SdbMutex

#include <memory>

#include "all_src.h"
#include "doctest.h"

class BlinkHandlerTest : public SdbBlinkMode::BlinkHandler {
public:
    BlinkHandlerTest() :
            SdbBlinkMode::BlinkHandler([this]() -> SdbBlinkMode::Mode {
                auto mode = _nextMode;
                _nextMode = SdbBlinkMode::Undefined;
                captureMode(mode);
                return mode;
            }),
            _nextMode(SdbBlinkMode::Undefined),
            _onboardLED(false),
            _externalLED(false),
            _clockMs(0)
    { }

    void setNextMode(SdbBlinkMode::Mode mode) {
        _nextMode = mode;
    }

    millis_t clockMs() {
        return _clockMs;
    }

    std::string actions() {
        std::string str = "";
        for (const auto &s: _actions) {
            str += s;
            str += '\n';
        }
        return str;
    }

protected:
    void setOnboardLED(bool on) override {
        _onboardLED = on;
    }

    void setExternalLED(bool on) override {
        _externalLED = on;
    }

    void sleepMs(millis_t delayMs) override {
        captureLED();
        _clockMs += delayMs;
    }

private:
    SdbBlinkMode::Mode _nextMode;
    millis_t _clockMs;
    bool _onboardLED;
    bool _externalLED;
    std::vector<std::string> _actions;

    void captureLED() {
        char buf[40];
        snprintf(buf, sizeof(buf), "%06ld ms: LED (%c) (%c)",
                 _clockMs,
                 _onboardLED ? 'O' : 'x',
                 _externalLED ? 'E' : 'x');
        _actions.emplace_back(buf);
    }

    void captureMode(SdbBlinkMode::Mode mode) {
        char buf[40];
        snprintf(buf, sizeof(buf), "%06ld ms: read mode 0x%04X",
                 _clockMs,
                 mode);
        _actions.emplace_back(buf);
    }
};


TEST_SUITE_BEGIN("SdbBlinkMode");

TEST_CASE("SdbBlinkMode idle") {
    auto handler = BlinkHandlerTest();

    CHECK_EQ(handler.actions(), "");
    CHECK_EQ(handler.clockMs(), 0);
    CHECK_EQ(handler.currentMode(), SdbBlinkMode::Undefined);
}

TEST_CASE("SdbBlinkMode idle") {
    auto handler = BlinkHandlerTest();

    handler.onLoop();
    handler.onLoop();

    CHECK_EQ(handler.actions(),
             "000000 ms: read mode 0xFFFF\n"
             "000000 ms: LED (x) (x)\n"
             "000250 ms: read mode 0xFFFF\n"
             "000250 ms: LED (x) (x)\n");

    CHECK_EQ(handler.clockMs(), 500);
    CHECK_EQ(handler.currentMode(), SdbBlinkMode::Undefined);
}

TEST_CASE("SdbBlinkMode AlwaysOn") {
    auto handler = BlinkHandlerTest();

    handler.setNextMode(SdbBlinkMode::AlwaysOn);
    handler.onLoop();
    // Mode stays always on and is not reset.

    CHECK_EQ(handler.actions(),
             "000000 ms: read mode 0x0808\n"
             "000000 ms: LED (O) (E)\n"
             "000250 ms: LED (O) (E)\n");

    CHECK_EQ(handler.clockMs(), 500);
    CHECK_EQ(handler.currentMode(), SdbBlinkMode::AlwaysOn);
}

TEST_CASE("SdbBlinkMode AlwaysOff") {
    auto handler = BlinkHandlerTest();

    handler.setNextMode(SdbBlinkMode::AlwaysOff);
    handler.onLoop();
    // Mode stays always off and is not reset.

    CHECK_EQ(handler.actions(),
             "000000 ms: read mode 0x0000\n"
             "000000 ms: LED (x) (x)\n"
             "000250 ms: LED (x) (x)\n");

    CHECK_EQ(handler.clockMs(), 500);
    CHECK_EQ(handler.currentMode(), SdbBlinkMode::AlwaysOff);
}

TEST_CASE("SdbBlinkMode APBoot") {
    auto handler = BlinkHandlerTest();

    handler.setNextMode(SdbBlinkMode::APBoot);
    handler.onLoop();
    handler.onLoop();

    CHECK_EQ(handler.actions(),
             "000000 ms: read mode 0x3008\n"
             "000000 ms: LED (O) (E)\n"
             "001000 ms: LED (x) (E)\n"
             "002000 ms: read mode 0xFFFF\n"
             "002000 ms: LED (O) (E)\n"
             "003000 ms: LED (x) (E)\n");

    CHECK_EQ(handler.clockMs(), 4000);
    CHECK_EQ(handler.currentMode(), SdbBlinkMode::APBoot);
}

TEST_CASE("SdbBlinkMode APConnectedOK") {
    auto handler = BlinkHandlerTest();

    handler.setNextMode(SdbBlinkMode::APConnectedOK);
    handler.onLoop();
    handler.onLoop();

    CHECK_EQ(handler.actions(),
             "000000 ms: read mode 0x2008\n"
             "000000 ms: LED (O) (E)\n"
             "002000 ms: LED (x) (E)\n"
             "004000 ms: read mode 0xFFFF\n"
             "004000 ms: LED (O) (E)\n"
             "006000 ms: LED (x) (E)\n");

    CHECK_EQ(handler.clockMs(), 8000);
    CHECK_EQ(handler.currentMode(), SdbBlinkMode::APConnectedOK);
}

TEST_CASE("SdbBlinkMode APFatalError") {
    auto handler = BlinkHandlerTest();

    handler.setNextMode(SdbBlinkMode::APFatalError);
    handler.onLoop();
    handler.onLoop();

    CHECK_EQ(handler.actions(),
             "000000 ms: read mode 0x2020\n"
             "000000 ms: LED (O) (E)\n"
             "002000 ms: LED (x) (x)\n"
             "004000 ms: read mode 0xFFFF\n"
             "004000 ms: LED (O) (E)\n"
             "006000 ms: LED (x) (x)\n");

    CHECK_EQ(handler.clockMs(), 8000);
    CHECK_EQ(handler.currentMode(), SdbBlinkMode::APFatalError);
}

TEST_CASE("SdbBlinkMode STABoot") {
    auto handler = BlinkHandlerTest();

    handler.setNextMode(SdbBlinkMode::STABoot);
    handler.onLoop();
    handler.onLoop();

    CHECK_EQ(handler.actions(),
             "000000 ms: read mode 0x3000\n"
             "000000 ms: LED (O) (x)\n"
             "001000 ms: LED (x) (x)\n"
             "002000 ms: read mode 0xFFFF\n"
             "002000 ms: LED (O) (x)\n"
             "003000 ms: LED (x) (x)\n");

    CHECK_EQ(handler.clockMs(), 4000);
    CHECK_EQ(handler.currentMode(), SdbBlinkMode::STABoot);
}

TEST_CASE("SdbBlinkMode STAConnectedOk") {
    auto handler = BlinkHandlerTest();

    handler.setNextMode(SdbBlinkMode::STAConnectedOk);
    handler.onLoop();
    handler.onLoop();

    CHECK_EQ(handler.actions(),
             "000000 ms: read mode 0x2000\n"
             "000000 ms: LED (O) (x)\n"
             "002000 ms: LED (x) (x)\n"
             "004000 ms: read mode 0xFFFF\n"
             "004000 ms: LED (O) (x)\n"
             "006000 ms: LED (x) (x)\n");

    CHECK_EQ(handler.clockMs(), 8000);
    CHECK_EQ(handler.currentMode(), SdbBlinkMode::STAConnectedOk);
}

TEST_CASE("SdbBlinkMode STAMeasureOK") {
    auto handler = BlinkHandlerTest();

    handler.setNextMode(SdbBlinkMode::STAMeasureOK);
    handler.onLoop();
    handler.onLoop();

    CHECK_EQ(handler.actions(),
             "000000 ms: read mode 0x0100\n"
             "000000 ms: LED (O) (x)\n"
             "000250 ms: LED (x) (x)\n"
             "000500 ms: read mode 0xFFFF\n"
             "000500 ms: LED (x) (x)\n"
             "000750 ms: LED (x) (x)\n");

    CHECK_EQ(handler.clockMs(), 1000);
    CHECK_EQ(handler.currentMode(), SdbBlinkMode::STAIdle);
}

TEST_CASE("SdbBlinkMode STAMeasureFail") {
    auto handler = BlinkHandlerTest();

    handler.setNextMode(SdbBlinkMode::STAMeasureFail);
    handler.onLoop();
    handler.onLoop();

    CHECK_EQ(handler.actions(),
             "000000 ms: read mode 0x0001\n"
             "000000 ms: LED (x) (E)\n"
             "000250 ms: LED (x) (x)\n"
             "000500 ms: read mode 0xFFFF\n"
             "000500 ms: LED (x) (x)\n"
             "000750 ms: LED (x) (x)\n");

    CHECK_EQ(handler.clockMs(), 1000);
    CHECK_EQ(handler.currentMode(), SdbBlinkMode::STAIdle);
}

TEST_CASE("SdbBlinkMode STAPublishFail") {
    auto handler = BlinkHandlerTest();

    handler.setNextMode(SdbBlinkMode::STAPublishFail);
    handler.onLoop();
    handler.onLoop();

    CHECK_EQ(handler.actions(),
             "000000 ms: read mode 0x0101\n"
             "000000 ms: LED (O) (E)\n"
             "000250 ms: LED (x) (x)\n"
             "000500 ms: read mode 0xFFFF\n"
             "000500 ms: LED (x) (x)\n"
             "000750 ms: LED (x) (x)\n");

    CHECK_EQ(handler.clockMs(), 1000);
    CHECK_EQ(handler.currentMode(), SdbBlinkMode::STAIdle);
}

TEST_CASE("SdbBlinkMode STAFatalError") {
    auto handler = BlinkHandlerTest();

    handler.setNextMode(SdbBlinkMode::STAFatalError);
    handler.onLoop();
    handler.onLoop();

    CHECK_EQ(handler.actions(),
             "000000 ms: read mode 0x1010\n"
             "000000 ms: LED (O) (E)\n"
             "004000 ms: LED (x) (x)\n"
             "008000 ms: read mode 0xFFFF\n"
             "008000 ms: LED (O) (E)\n"
             "012000 ms: LED (x) (x)\n");

    CHECK_EQ(handler.clockMs(), 16000);
    CHECK_EQ(handler.currentMode(), SdbBlinkMode::STAFatalError);
}


TEST_SUITE_END();
