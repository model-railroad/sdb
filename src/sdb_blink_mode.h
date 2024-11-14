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

#pragma once

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

//---------------
