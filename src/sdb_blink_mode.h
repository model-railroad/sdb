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

#ifndef INC_SDB_BLINK_MODE_H
#define INC_SDB_BLINK_MODE_H

#include "common.h"

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
#define BLINK_ON                    0xFF
#define BLINK_FLASH_VERY_SLOWLY     0x10
#define BLINK_FLASH_SLOWLY          0x20
#define BLINK_FLASH_RAPIDLY         0x30
#define BLINK_ONCE                  0x01
#define BLINK_OFF                   0x00

namespace SdbBlinkMode {
    enum Mode {
        Undefined       = 0,
        APBoot          = BLINK_FLASH_RAPIDLY << 8      | BLINK_ON,
        APConnectedOK   = BLINK_FLASH_SLOWLY << 8       | BLINK_ON,
        APFatalError    = BLINK_FLASH_SLOWLY << 8       | BLINK_FLASH_SLOWLY,
        STABoot         = BLINK_FLASH_RAPIDLY << 8      | BLINK_OFF,
        STAConnectedOk  = BLINK_FLASH_SLOWLY << 8       | BLINK_OFF,
        STAIdle         = BLINK_OFF << 8                | BLINK_OFF,
        STAMeasureOK    = BLINK_ONCE << 8               | BLINK_OFF,
        STAMeasureFail  = BLINK_OFF << 8                | BLINK_ONCE,       // Next: STAIdle
        STAPublishOk    = BLINK_ONCE << 8               | BLINK_OFF,        // Next: STAIdle
        STAPublishFail  = BLINK_ONCE << 8               | BLINK_ONCE,       // Next: STAIdle
        STAFatalError   = BLINK_FLASH_VERY_SLOWLY << 8  | BLINK_FLASH_VERY_SLOWLY,
    };
}


//---------------

#endif // INC_SDB_BLINK_MODE_H
