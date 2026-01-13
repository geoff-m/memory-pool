#include "gtest/gtest.h"
#include "memory_pool.h"
#include "TestUtils.h"

TEST(SingleThread, Create) {
    memory_pool pool(10000);
}

TEST(SingleThread, UseAtOnce) {
    constexpr auto size = 10000;
    memory_pool pool(size);
    useMemory(pool.allocate(size), size);
    assertPoolFull(pool);
}

TEST(SingleThread, Use) {
    constexpr auto size = 10000;
    memory_pool pool(size, false, false, out_of_memory_behavior::ReturnNull);
    EXPECT_EQ(pool.get_capacity(), size);
    constexpr auto chunkSize = 1234;
    constexpr auto expectedChunkCount = size / chunkSize;
    int chunkCount = 0;
    while (pool.get_size() < pool.get_capacity()) {
        auto* chunk = pool.allocate(chunkSize);
        if (chunk == nullptr) {
            EXPECT_EQ(expectedChunkCount, chunkCount);
            break;
        }
        memset(chunk, 0, chunkSize);
        ++chunkCount;
    }
    EXPECT_EQ(pool.get_capacity(), size);
    EXPECT_EQ(expectedChunkCount, chunkCount);
    EXPECT_EQ(pool.get_size(), expectedChunkCount * chunkSize);
}

TEST(SingleThread, AllocateReturnsNull) {
    memory_pool pool(150, false, false, out_of_memory_behavior::ReturnNull);
    useMemory(pool.allocate(100), 100);
    EXPECT_EQ(nullptr, pool.allocate(100));
    useMemory(pool.allocate(50), 50);
    assertPoolFull(pool);
}

TEST(SingleThread, AllocateThrows) {
    memory_pool pool(150, false, false, out_of_memory_behavior::Throw);
    useMemory(pool.allocate(100), 100);
    EXPECT_ANY_THROW((void)pool.allocate(100));
    useMemory(pool.allocate(50), 50);
    assertPoolFull(pool);
}

TEST(SingleThread, TemplateAllocate) {
    using T1 = char;
    using T2 = long long;
    constexpr auto t1Size = sizeof(T1);
    constexpr auto t2Size = sizeof(T2);
    memory_pool pool(t1Size + t2Size, false, false, out_of_memory_behavior::Throw);
    useMemory(pool.allocate<T1>(), t1Size);
    useMemory(pool.allocate<T2>(), t2Size);
    assertPoolFull(pool);
}

struct Foo {
    const int x;
    const int y;

    Foo(int x, int y) : x(x), y(y) {
    }
};

TEST(SingleThread, TemplateAllocateConstrArgs) {
    memory_pool pool(sizeof(Foo), false, false, out_of_memory_behavior::Throw);
    auto* foo = pool.allocate<Foo>(10, 20);
    EXPECT_EQ(10, foo->x);
    EXPECT_EQ(20, foo->y);
    useMemory(foo, sizeof(Foo));
    assertPoolFull(pool);
}
