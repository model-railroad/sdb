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

#include "all_src.h"
#include "doctest.h"

TEST_SUITE_BEGIN("SdbLock");

TEST_CASE("SdbLock acquire release") {
    SdbLock lock("name");

    CHECK(gLastSemaphore != nullptr);
    CHECK_EQ(gLastSemaphore->counter, 0);

    lock.acquire();
    CHECK_EQ(gLastSemaphore->counter, 1);

    lock.release();
    CHECK_EQ(gLastSemaphore->counter, 0);
}

TEST_CASE("SdbLock execute") {
    SdbLock lock("name");

    lock.execute([&] {
        CHECK_EQ(gLastSemaphore->counter, 1);
    });

    CHECK_EQ(gLastSemaphore->counter, 0);
}

TEST_CASE("SdbMutex") {
    SdbLock lock("name");

    CHECK_EQ(gLastSemaphore->counter, 0);

    {
        SdbMutex mutex(lock);
        CHECK_EQ(gLastSemaphore->counter, 1);
    }

    CHECK_EQ(gLastSemaphore->counter, 0);
}

TEST_SUITE_END();
