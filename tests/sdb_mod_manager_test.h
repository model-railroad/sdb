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

// Test SdbLock and SdbMutex

#include <memory>

#include "all_src.h"
#include "doctest.h"

TEST_SUITE_BEGIN("SdbModManager");

TEST_CASE("SdbModManager register mod") {
    SdbModManager manager;
    SdbModTest mod(manager, "SdbModTest");
    manager.onStart();

    manager.registerMod(&mod);

    CHECK_EQ(manager.modByName("SdbModTest"), &mod);

    CHECK_EQ(manager.modByName("UnknownMod"), nullptr);
}

TEST_CASE("SdbModManager schedule") {
    SdbModManager manager;
    manager.onStart();

    int action = 0;

    millis_t startMs = millis();

    manager.schedule(500L, [&] {
        action = 1;
    });

    manager.schedule(750L, [&] {
        action = 2;
    });

    // No action executed yet (until onLoop is called)
    CHECK_EQ(action, 0);
    CHECK_EQ(millis(), startMs);

    // First action will only happen in 500ms in the future.
    // However, this sets the loop's next delay to that time,
    // and by side effect sets the global millis() to that event.
    manager.onLoop();
    CHECK_EQ(action, 0);
    CHECK_EQ(millis(), startMs + 500);

    // Next action is at startMs + 750, so go past it to startMs + 500 + 500.
    delay(500L);
    manager.onLoop();
    CHECK_EQ(action, 2);

    // The loop is now idle and won't check again for 2 seconds.
    CHECK_EQ(millis(), startMs + 500 + 500 + 2000);
}

TEST_SUITE_END();
