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

#pragma once

// Wifi module.
// AP mode, a.k.a. "Ad-hoc wifi": this module generates its own wifi network,
//   and clients can connect to it to provide the initial configuration.
// STA mode, a.k.a. "normal wifi": this module connects to an existing wifi network,
//   provides pages for configuration, and sent its state to a JMIR or MQTT server.

#define MOD_WIFI_NAME "wi"

