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

#ifndef INC_SDB_PROPS_H
#define INC_SDB_PROPS_H

#include "common.h"
#include <Arduino_JSON.h>

//-----------------------------------

static const JSONVar& mkProp(JSONVar& var, const char* label, const String& val) {
    var["l"] = label;
    var["v"] = val.c_str();
    return var;
}


#endif // INC_SDB_PROPS_H
