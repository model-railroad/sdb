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

#define ESP32 1

#include "esp.h"
#include "common.h"
#include "sdb_ino.h"

// No display in the mock tests (TBD add later)
#undef USE_DISPLAY_LIB_U8G2

// These includes are specifically in the same order as the ones in sdb.ino.
// The order matters.
#include "sdb_mod_manager.h"
#include "mod_blinky.h"
#include "mod_blocks.h"
#include "mod_display.h"
#include "mod_jmri.h"
#include "mod_mqtt.h"
#include "mod_tof.h"
#include "mod_wifi.h"

// Include every sdb_* and every mod_ to make sure they compile.
#include "sdb_blink_mode.h"
#include "sdb_block.h"
#include "sdb_data_store.h"
#include "sdb_event.h"
#include "sdb_lock.h"
#include "sdb_mod.h"
#include "sdb_mod_manager.h"
#include "sdb_pass_dec.h"
#include "sdb_props.h"
#include "sdb_sensor.h"
#include "sdb_server.h"
#include "sdb_task.h"

#include "mod_blinky.h"
#include "mod_blocks.h"
#include "mod_display.h"
#include "mod_jmri.h"
#include "mod_mqtt.h"
#include "mod_tof.h"
#include "mod_wifi.h"
