#include "gtest/gtest.h"
#include "memory_pool.h"
#include "TestUtils.h"

using namespace memory_pool;

TEST(SingleThread, Create) {
    auto* pool = pool::create(10000);
    delete pool;
}

TEST(SingleThread, UseAtOnce) {
    constexpr auto size = 10000;
    auto* pool = pool::create(size);
    useMemory(pool->allocate(size), size);
    assertPoolFull(*pool);
    delete pool;
}

TEST(SingleThread, UseAtOnceBig) {
    constexpr auto size = 1024*1024 * 10;
    auto* pool = pool::create(size);
    useMemory(pool->allocate(size), size);
    assertPoolFull(*pool);
    delete pool;
}

TEST(SingleThread, Use) {
    constexpr auto size = 10000;
    auto* pool = pool::create(size, pool_type::SingleThreaded, out_of_memory_behavior::ReturnNull);
    EXPECT_EQ(pool->get_capacity(), size);
    constexpr auto chunkSize = 1234;
    constexpr auto expectedChunkCount = size / chunkSize;
    int chunkCount = 0;
    while (pool->get_size() < pool->get_capacity()) {
        auto* chunk = pool->allocate(chunkSize);
        if (chunk == nullptr) {
            EXPECT_EQ(expectedChunkCount, chunkCount);
            break;
        }
        memset(chunk, 0, chunkSize);
        ++chunkCount;
    }
    EXPECT_EQ(pool->get_capacity(), size);
    EXPECT_EQ(expectedChunkCount, chunkCount);
    EXPECT_EQ(pool->get_size(), expectedChunkCount * chunkSize);
    delete pool;
}

TEST(SingleThread, AllocateReturnsNull) {
    auto* pool = pool::create(150, pool_type::SingleThreaded, out_of_memory_behavior::ReturnNull);
    useMemory(pool->allocate(100), 100);
    EXPECT_EQ(nullptr, pool->allocate(100));
    useMemory(pool->allocate(50), 50);
    assertPoolFull(*pool);
    delete pool;
}

TEST(SingleThread, AllocateThrows) {
    auto* pool = pool::create(150, pool_type::SingleThreaded, out_of_memory_behavior::Throw);
    useMemory(pool->allocate(100), 100);
    EXPECT_ANY_THROW((void)pool->allocate(100));
    useMemory(pool->allocate(50), 50);
    assertPoolFull(*pool);
    delete pool;
}

TEST(SingleThread, TemplateAllocate) {
    using T1 = char;
    using T2 = long long;
    constexpr auto t1Size = sizeof(T1);
    constexpr auto t2Size = sizeof(T2);
    auto* pool = pool::create(t1Size + t2Size, pool_type::SingleThreaded, out_of_memory_behavior::Throw);
    useMemory(pool->allocate<T1>(), t1Size);
    useMemory(pool->allocate<T2>(), t2Size);
    assertPoolFull(*pool);
    delete pool;
}

struct Foo {
    const int x;
    const int y;

    Foo(int x, int y) : x(x), y(y) {
    }
};

TEST(SingleThread, TemplateAllocateConstrArgs) {
    auto* pool = pool::create(sizeof(Foo), pool_type::SingleThreaded, out_of_memory_behavior::Throw);
    auto* foo = pool->allocate<Foo>(10, 20);
    EXPECT_EQ(10, foo->x);
    EXPECT_EQ(20, foo->y);
    useMemory(foo, sizeof(Foo));
    assertPoolFull(*pool);
    delete pool;
}
