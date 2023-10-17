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

#ifndef INC_CONFIG_H
#define INC_CONFIG_H

#if !defined(ESP32)
    #warning ESP32 is not defined. Defining it here.
    #define ESP32 1
#endif

// Graphics Library being used (if any)
#define USE_DISPLAY_LIB_U8G2

// CPU affinity for ESP32
#define PRO_CPU 0       // Wifi
#define APP_CPU 1       // Main app


#endif // INC_CONFIG_H
