// Test SdbLock and SdbMutex

#include "all_src.h"
#include "doctest.h"


TEST_CASE("sdbPassDec") {
    auto p = sdbPassDec("414243646566303132");
    CHECK_EQ(p, "ABCdef012");
}
