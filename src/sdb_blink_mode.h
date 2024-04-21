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
#define BLINK_FLASH_SLOWLY_3_TIMES  0x23
#define BLINK_FLASH_RAPIDLY         0x30
#define BLINK_ONCE                  0x01
#define BLINK_OFF                   0x00

enum SdbBlinkMode {
    BLINK_UNSET = 0,
    BLINK_AP_BOOT               = BLINK_FLASH_RAPIDLY << 8          | BLINK_ON,
    BLINK_AP_CONNECTED_OK       = BLINK_FLASH_SLOWLY << 8           | BLINK_ON,
    BLINK_AP_FATAL_ERROR        = BLINK_FLASH_SLOWLY << 8           | BLINK_FLASH_SLOWLY,
    BLINK_STA_BOOT              = BLINK_FLASH_RAPIDLY << 8          | BLINK_OFF,
    BLINK_STA_CONNECTED_OK      = BLINK_FLASH_SLOWLY_3_TIMES << 8   | BLINK_OFF,
    BLINK_STA_IDLE              = BLINK_OFF << 8                    | BLINK_OFF,
    BLINK_STA_MEASURE_OK        = BLINK_ONCE << 8                   | BLINK_OFF,
    BLINK_STA_MEASURE_FAIL      = BLINK_OFF << 8                    | BLINK_ONCE,
    BLINK_STA_PUBLISH_OK        = BLINK_ONCE << 8                   | BLINK_OFF,
    BLINK_STA_PUBLISH_FAIL      = BLINK_ONCE << 8                   | BLINK_ONCE,
    BLINK_STA_FATAL_ERROR       = BLINK_FLASH_VERY_SLOWLY << 8      | BLINK_FLASH_VERY_SLOWLY,
};


//---------------

#endif // INC_SDB_BLINK_MODE_H
