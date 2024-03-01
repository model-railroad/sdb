// Test SdbLock and SdbMutex

#include "all_src.h"
#include "doctest.h"

class SdbModTest : public SdbMod {
public:
    SdbModTest(SdbModManager& manager, const String& name) :
            SdbMod(manager, name)
    { }

    bool hasEvents() {
        return SdbMod::hasEvents();
    }

    SdbEvent::SdbEvent dequeueEvent() {
        return SdbMod::dequeueEvent();
    }
};

TEST_CASE("SdbMod name") {
    SdbModManager manager;
    SdbModTest mod(manager, "SDB Mod Test Name");

    CHECK_EQ(mod.name(), "SDB Mod Test Name");
}

TEST_CASE("SdbMod events") {
    SdbModManager manager;
    SdbModTest mod(manager, "SDB Mod Test Name");

    CHECK_FALSE(mod.hasEvents());

    auto event0 = mod.dequeueEvent();
    CHECK_EQ(event0, SdbEvent::EMPTY);


    mod.queueEvent(SdbEvent::DisplayWifiAP);
    mod.queueEvent(SdbEvent::DisplayWifiSTA);
    mod.queueEvent(SdbEvent::DisplaySensor);
    String blockName("Block Name");
    mod.queueEvent(SdbEvent::SdbEvent(SdbEvent::BlockChanged, true, &blockName));

    CHECK(mod.hasEvents());

    auto event1 = mod.dequeueEvent();
    CHECK_EQ(event1.type, SdbEvent::DisplayWifiAP);
    auto event2 = mod.dequeueEvent();
    CHECK_EQ(event2.type, SdbEvent::DisplayWifiSTA);
    auto event3 = mod.dequeueEvent();
    CHECK_EQ(event3.type, SdbEvent::DisplaySensor);
    auto event4 = mod.dequeueEvent();
    CHECK_EQ(event4.type, SdbEvent::BlockChanged);
    CHECK_EQ(event4.state, true);
    CHECK_EQ(event4.data->c_str(), "Block Name");

    auto event5 = mod.dequeueEvent();
    CHECK_EQ(event5.type, SdbEvent::Empty);

    CHECK_FALSE(mod.hasEvents());
}

