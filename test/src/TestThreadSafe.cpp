#include <gtest/gtest.h>
#include "TestUtils.h"
#include <thread>

using namespace memory_pool;

TEST(ThreadSafe, TryRace) {
    auto* pool = pool::create(2000, pool_type::ThreadSafe);
    std::thread t1([pool] {
        for (int i = 0; i < 1000; i++)
            useMemory(pool->do_allocate(1), 1);
    });
    std::thread t2([pool] {
        for (int i = 0; i < 1000; i++)
            useMemory(pool->do_allocate(1), 1);
    });
    t1.join();
    t2.join();
    assertPoolFull(*pool);
    delete pool;
}

TEST(ThreadSafe, TryRaceBig) {
    const auto MB = 1024 * 1024;
    auto* pool = pool::create(100 * MB, pool_type::ThreadSafe);
    constexpr auto allocationCount = 50;
    std::thread t1([pool] {
        for (int i = 0; i < allocationCount; i++) {
            useMemory(pool->do_allocate(MB), MB);
        }
    });
    std::thread t2([pool] {
        for (int i = 0; i < allocationCount; i++) {
            useMemory(pool->do_allocate(MB), MB);
        }
    });
    t1.join();
    t2.join();
    assertPoolFull(*pool);
    delete pool;
}

TEST(ThreadSafe, TryRacePerThread) {
    auto* pool = pool::create(1000, pool_type::PerThread);
    std::thread t1([pool] {
        for (int i = 0; i < 1000; i++)
            useMemory(pool->do_allocate(1), 1);
        assertPoolFull(*pool);
    });
    std::thread t2([pool] {
        for (int i = 0; i < 1000; i++)
            useMemory(pool->do_allocate(1), 1);
        assertPoolFull(*pool);
    });
    t1.join();
    t2.join();
    ASSERT_EQ(0, pool->get_size());
    delete pool;
}

TEST(ThreadSafe, TryRaceBigPerThread) {
    const auto MB = 1024 * 1024;
    auto* pool = pool::create(100 * MB, pool_type::PerThread);
    std::thread t1([pool] {
        for (int i = 0; i < 100; i++)
            useMemory(pool->do_allocate(MB), MB);
        assertPoolFull(*pool);
    });
    std::thread t2([pool] {
        for (int i = 0; i < 100; i++)
            useMemory(pool->do_allocate(MB), MB);
        assertPoolFull(*pool);
    });
    t1.join();
    t2.join();
    ASSERT_EQ(0, pool->get_size());
    delete pool;
}
