// Test SdbLock and SdbMutex

#include "all_src.h"
#include "doctest.h"

class SdbModTest : public SdbMod {
public:
    SdbModTest(SdbModManager& manager, const String& name) :
            SdbMod(manager, name)
    { }

};

TEST_CASE("SdbMod init") {
    SdbModManager manager;
    SdbModTest mod(manager, "SDB Mod Test Name");

    CHECK_EQ(mod.name(), "SDB Mod Test Name");
}

