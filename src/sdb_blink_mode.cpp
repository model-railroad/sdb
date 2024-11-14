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

module;

#include "common.h"
#include <functional>

#include "sdb_blink_mode.h"

//---------------
export module SDB.BlinkMode;

// TBD move this to header and inject in common
/*
 BlinkMode is an ESP32 32-bit enum value which encodes the flash pattern + flash counts.
 - Low 8-bits encode the External LED state.
 - Mid 8-bits encode the Onboard LED state.
 Each LED state is composed of 4-bits up = flash rate divider || 4-bits low = repeat count.
 0xFF is a special case for "always on" (error cases).
 The "repeat count" is zero for permanent states.
 When non-zero, the next step is always STA_IDLE, so we don’t need to encode it.
 The flash rate divider is a bit shift. Base constant is 8 seconds.
 - 8 >> 1 = 4	-- "very slow" (that’s the length of the 50% cycle in seconds)
 - 8 >> 2 = 2	-- "slowly"
 - 8 >> 3 = 1	-- "rapidly"
 */
#define BLINK_ON                    0x08
#define BLINK_FLASH_VERY_SLOWLY     0x10
#define BLINK_FLASH_SLOWLY          0x20
#define BLINK_FLASH_RAPIDLY         0x30
#define BLINK_ONCE                  0x01
#define BLINK_OFF                   0x00

#define BLINK_MASK                  0xFF
#define BLINK_COUNT_MASK            0x07
#define BLINK_FLASH_MASK            0xF0
#define BLINK_FLASH_OFFSET          4
#define BLINK_ONBOARD_OFFSET        8

#define BLINK_FLASH_LONG_CYCLE_MS  (8*1000)
#define BLINK_ONCE_MS               250
#define BLINK_LED_ONBOARD           0
#define BLINK_LED_EXTERNAL          1

export namespace SdbBlinkMode {
    enum Mode {
        Undefined       = 0xFFFF,
        AlwaysOn        = BLINK_ON << 8                 | BLINK_ON,
        AlwaysOff       = BLINK_OFF << 8                | BLINK_OFF,
        APBoot          = BLINK_FLASH_RAPIDLY << 8      | BLINK_ON,
        APConnectedOK   = BLINK_FLASH_SLOWLY << 8       | BLINK_ON,
        APFatalError    = BLINK_FLASH_SLOWLY << 8       | BLINK_FLASH_SLOWLY,
        STABoot         = BLINK_FLASH_RAPIDLY << 8      | BLINK_OFF,
        STAConnectedOk  = BLINK_FLASH_SLOWLY << 8       | BLINK_OFF,
        STAIdle         = BLINK_OFF << 8                | BLINK_OFF,
        STAMeasureOK    = BLINK_ONCE << 8               | BLINK_OFF,        // Next: STAIdle
        STAMeasureFail  = BLINK_OFF << 8                | BLINK_ONCE,       // Next: STAIdle
        STAPublishOk    = BLINK_ONCE << 8               | BLINK_OFF,        // Next: STAIdle
        STAPublishFail  = BLINK_ONCE << 8               | BLINK_ONCE,       // Next: STAIdle
        STAFatalError   = BLINK_FLASH_VERY_SLOWLY << 8  | BLINK_FLASH_VERY_SLOWLY,
    };

    class BlinkHandler {
    public:
        explicit BlinkHandler(const std::function<Mode()>& getNextMode) :
                _currentMode(Undefined),
                _getNextMode(getNextMode)
        { }

        /**
         * Called repeatedly from an infinite task. Processes blink mode events
         * and pauses as required. The function may pause for indeterminate amounts
         * of time as required by the current blink pattern.
         */
        void onLoop() {
            auto newMode = _getNextMode();
            if (newMode != Undefined && newMode != _currentMode) {
                _currentMode = newMode;
            }

            if (_currentMode != Undefined) {
                // The apply() implementation always has a minimum sleep time,
                // which is currently at least 2 x 250 ms.
                apply(_currentMode);
            } else {
                // Avoid busy looping.
                sleepMs(250);
            }
        }

        Mode currentMode() { return _currentMode; }

    protected:
        /// Implemented by adapter to turn the onboard LED on/off.
        virtual void setOnboardLED(bool on) = 0;

        /// Implemented by adapter to turn the external LED on/off.
        virtual void setExternalLED(bool on) = 0;

        /// Implemented by adapter to sleep expected delay.
        virtual void sleepMs(millis_t delayMs) = 0;

    private:
        const std::function<Mode()> _getNextMode;
        Mode _currentMode;

        void setLED(int ledIndex, bool on) {
            switch (ledIndex) { // NOLINT(*-multiway-paths-covered)
                case BLINK_LED_ONBOARD:
                    setOnboardLED(on);
                    break;
                case BLINK_LED_EXTERNAL:
                    setExternalLED(on);
                    break;
            }
        }

        void apply(Mode mode) {
            VERBOSE_PRINTF( ( "[BLINK] ---> APPLY mode %04X.\n", mode ) );

            const int externalMode = mode & BLINK_MASK;
            const int onboardMode = (mode >> BLINK_ONBOARD_OFFSET) & BLINK_MASK;

            const int externalCount   = externalMode & BLINK_COUNT_MASK;
            const int onboardCount    = onboardMode & BLINK_COUNT_MASK;

            // If either LED has a count:
            //  - blink using the count number.
            //  * cycle on for BLINK_ONCE_MS + cycle off same constant.
            //  * each LED is either on/off or off/off or on/on.
            //  - block till end, then _currentMode = STAIdle.
            //
            // Simplification: Only one LED has a count, or they are both the same.

            // If no count, this is an "infinite" mode.
            //  - currently only one side flashes *or* both at the same rate, so we can simplify.
            //  - compute half cycle length using 8 seconds >> divider.
            //  * cycle on for that time + cycle off same time.
            //  * each LED is either on/off or off/off or on/on.
            //  - block till end of cycle and just loop.
            //
            // Simplification: Only one LED flashing or they both have the same rate.

            const int maxCount = MAX(externalCount, onboardCount);

            const int onboardFlash  = (onboardMode  & BLINK_FLASH_MASK) >> BLINK_FLASH_OFFSET;
            const int externalFlash = (externalMode & BLINK_FLASH_MASK) >> BLINK_FLASH_OFFSET;
            const int maxFlash = MAX(onboardFlash, externalFlash);
            const int durationMs = maxFlash == 0 ? BLINK_ONCE_MS : (BLINK_FLASH_LONG_CYCLE_MS >> maxFlash);

            applyCount(durationMs,
                       maxCount,
                       onboardMode,
                       externalMode);

            if (externalCount > 0 || onboardCount > 0) {
                _currentMode = STAIdle;
            }
        }

        void applyCount(int durationMs, int cycleCount, int onboardMode, int externalMode) {
            // Note: This always does at least one iteration, to ensure we can process the
            // always-off & always-on cases equally. This means this function always has a
            // sleep time of at least 2 x durationMs.
            int count = 0;
            do {
                applyPattern(BLINK_LED_ONBOARD, onboardMode, true);
                applyPattern(BLINK_LED_EXTERNAL, externalMode, true);
                sleepMs(durationMs);
                applyPattern(BLINK_LED_ONBOARD, onboardMode, false);
                applyPattern(BLINK_LED_EXTERNAL, externalMode, false);
                sleepMs(durationMs);
            } while (++count < cycleCount);
        }

        void applyPattern(int ledIndex, int mode, bool cycle) {
            bool state = cycle;
            if (mode == BLINK_ON) {
                state = true;
            } else if (mode == BLINK_OFF) {
                state = false;
            } else if ((mode & BLINK_COUNT_MASK) != 0 || (mode & BLINK_FLASH_MASK) != 0) {
                // state = cycle; -- nothing to do, this is the default value.
            }
            setLED(ledIndex, state);
        }

    };

}

//---------------
