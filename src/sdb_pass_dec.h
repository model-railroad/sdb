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

#ifndef INC_SDB_PASS_DEC_H
#define INC_SDB_PASS_DEC_H

// Wifi module.
// AP mode, a.k.a. "Ad-hoc wifi": this module generates its own wifi network,
//   and clients can connect to it to provide the initial configuration.
// STA mode, a.k.a. "normal wifi": this module connects to an existing wifi network,
//   provides pages for configuration, and sent its state to a JMIR or MQTT server.

#include "common.h"
#include "sdb_block.h"
#include "mod_display.h"
#include "sdb_lock.h"
#include "sdb_mod.h"
#include "sdb_sensor.h"
#include "sdb_server.h"

#include <algorithm>
#include <functional>
#include <vector>

#include <esp_http_server.h>
#include <Arduino_JSON.h>
#include <WiFi.h>
#include <WiFiClient.h>

#define MOD_WIFI_NAME "wi"

static inline char sdbPassDec_hex2int(char c) {
    if (c >= '0' && c <= '9') return      c - '0';
    if (c >= 'A' && c <= 'F') return 10 + c - 'A';
    if (c >= 'a' && c <= 'f') return 10 + c - 'a';
    return 0;
}

String sdbPassDec(const String& encodedPass) {
    const char *pw2src = encodedPass.c_str();
    int pwlen = encodedPass.length() / 2;
    std::unique_ptr<char[]> buffer(new char[pwlen + 1]);
    char* pwdst = buffer.get();
    for(int i = 0; i < pwlen; i++) {
        char c = (sdbPassDec_hex2int(*pw2src++) << 4) + sdbPassDec_hex2int(*pw2src++);
        pwdst[i] = c;
    }

    pwdst[pwlen] = 0;
    String pw(pwdst);
    return pw;
}

#endif // INC_SDB_PASS_DEC_H
