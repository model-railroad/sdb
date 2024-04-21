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

#ifndef INC_SDB_EVENT_H
#define INC_SDB_EVENT_H

#include "common.h"
#include "sdb_blink_mode.h"

namespace SdbEvent {
    enum Type {
        /// SdbEventBlinkMode with new blink mode.
        BlinkModeUpdated,
        /// SdbEvent with no extra data.
        DisplayWifiAP,
        /// SdbEvent with no extra data.
        DisplayWifiSTA,
        /// SdbEvent with no extra data.
        DisplaySensor,
        /// SdbEventBlockChanged with state and payload (String).
        BlockChanged,
    };

    class SdbEvent {
    public:
        explicit SdbEvent(Type type)
         : type(type)
        { }

        bool operator ==(const SdbEvent &rhs) const {
            return type == rhs.type;   // Note: String *pointer* equality
        }

        const Type type;
    };

    class SdbEventBlockChanged : public SdbEvent {
    public:
        SdbEventBlockChanged(bool state, String payload)
        : SdbEvent(BlockChanged), state(state), payload(std::move(payload))
        { }

        bool operator ==(const SdbEventBlockChanged &rhs) const {
            return type == rhs.type
                   && state == rhs.state
                   && payload == rhs.payload;
        }

        const bool state;
        const String payload;
    };

    class SdbEventBlinkMode : public SdbEvent {
    public:
        SdbEventBlinkMode(SdbBlinkMode blinkMode)
                : SdbEvent(BlinkModeUpdated), blinkMode(blinkMode)
        { }

        bool operator ==(const SdbEventBlinkMode &rhs) const {
            return type == rhs.type
                   && blinkMode == rhs.blinkMode;
        }

        const SdbBlinkMode blinkMode;
    };
}

#endif // INC_SDB_EVENT_H
