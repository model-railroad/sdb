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

class SdbModTest : public SdbMod {
public:
    SdbModTest(SdbModManager& manager, const String& name) :
            SdbMod(manager, name)
    { }

    // Visible for testing
    bool _hasEvents() {
        return SdbMod::hasEvents();
    }

    // Visible for testing
    std::unique_ptr<SdbEvent::SdbEvent> _dequeueEvent() {
        return std::move(SdbMod::dequeueEvent());
    }
};




TEST_SUITE_BEGIN("SdbMod");

TEST_CASE("SdbMod name") {
    SdbModManager manager;
    auto mod = std::make_shared<SdbModTest>(manager, "SdbModTestName");
    manager.registerMod(mod);

    CHECK_EQ(mod->name(), "SdbModTestName");
}

TEST_CASE("SdbMod events") {
    SdbModManager manager;
    auto mod = std::make_shared<SdbModTest>(manager, "SdbModTest");
    manager.registerMod(mod);

    CHECK_FALSE(mod->_hasEvents());

    auto event0 = mod->_dequeueEvent();
    CHECK_FALSE(event0);
    CHECK_EQ(event0.get(), nullptr);

    manager.queueEvent("SdbModTest", SdbEvent::DisplayWifiAP);
    manager.queueEvent("SdbModTest", SdbEvent::DisplayWifiSTA);
    manager.queueEvent("SdbModTest", SdbEvent::DisplaySensor);
    auto blockEvent = std::make_unique<SdbEvent::SdbEventBlockChanged>(
            true,
            "Block Name");
    manager.queueEvent("SdbModTest", std::move(blockEvent));

    CHECK(mod->_hasEvents());

    auto event1 = mod->_dequeueEvent();
    CHECK_EQ(event1->type, SdbEvent::DisplayWifiAP);
    auto event2 = mod->_dequeueEvent();
    CHECK_EQ(event2->type, SdbEvent::DisplayWifiSTA);
    auto event3 = mod->_dequeueEvent();
    CHECK_EQ(event3->type, SdbEvent::DisplaySensor);
    auto event4 = mod->_dequeueEvent();
    auto eventBlock = reinterpret_cast<SdbEvent::SdbEventBlockChanged *>(event4.get());
    CHECK_EQ(eventBlock->type, SdbEvent::BlockChanged);
    CHECK_EQ(eventBlock->state, true);
    CHECK_EQ(eventBlock->payload, "Block Name");
    auto event5 = mod->_dequeueEvent();
    CHECK_FALSE(event5);

    CHECK_FALSE(mod->_hasEvents());
}

TEST_SUITE_END();
