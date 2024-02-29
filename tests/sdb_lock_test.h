// Test SdbLock and SdbMutex

#include "all_src.h"
#include "doctest.h"


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
