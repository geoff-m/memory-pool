#include "gtest/gtest.h"
#include "memory-pool/memory_pool.h"
#include "TestUtils.h"

using namespace memory_pool;

TEST(SingleThread, Create) {
    auto* pool = pool::create(10000);
    delete pool;
}

TEST(SingleThread, UseAtOnce) {
    constexpr auto size = 10000;
    auto* pool = pool::create(size);
    useMemory(pool->new_buffer(size), size);
    assertPoolFull(*pool);
    delete pool;
}

TEST(SingleThread, UseAtOnceBig) {
    constexpr auto size = 1024 * 1024 * 10;
    auto* pool = pool::create(size);
    useMemory(pool->new_buffer(size), size);
    assertPoolFull(*pool);
    delete pool;
}

TEST(SingleThread, Use) {
    constexpr auto size = 10000;
    auto* pool = pool::create(size, pool_type::SingleThreaded);
    EXPECT_EQ(pool->get_capacity(), size);
    constexpr auto chunkSize = 1234;
    constexpr auto expectedChunkCount = size / chunkSize;
    int chunkCount = 0;
    while (pool->get_size() < pool->get_capacity()) {
        try {
            auto* chunk = pool->new_buffer(chunkSize);
            memset(chunk, 0, chunkSize);
            ++chunkCount;
        } catch (const std::exception& ex) {
            EXPECT_EQ(expectedChunkCount, chunkCount);
            break;
        }
    }
    EXPECT_EQ(pool->get_capacity(), size);
    EXPECT_EQ(expectedChunkCount, chunkCount);
    EXPECT_EQ(pool->get_size(), expectedChunkCount * chunkSize);
    delete pool;
}

TEST(SingleThread, AllocateThrows) {
    auto* pool = pool::create(150, pool_type::SingleThreaded);
    useMemory(pool->new_buffer(100), 100);
    EXPECT_ANY_THROW((void)pool->new_buffer(100));
    useMemory(pool->new_buffer(20), 20);
    delete pool;
}

TEST(SingleThread, TemplateAllocate) {
    using T1 = char;
    using T2 = long long;
    constexpr auto t1Size = sizeof(T1);
    constexpr auto t2Size = sizeof(T2);
    auto* pool = pool::create((t1Size + t2Size) * 2, pool_type::SingleThreaded);
    std::pmr::polymorphic_allocator<T1> t1Allocator(pool);
    useMemory(t1Allocator.new_object<T1>(), t1Size);
    std::pmr::polymorphic_allocator<T2> t2Allocator(pool);
    useMemory(t2Allocator.new_object<T2>(), t2Size);
    delete pool;
}

TEST(SingleThread, TemplateAllocate2) {
    using T1 = char;
    using T2 = long long;
    constexpr auto t1Size = sizeof(T1);
    constexpr auto t2Size = sizeof(T2);
    allocator<T1> t1Allocator((t1Size + t2Size) * 2, pool_type::SingleThreaded);
    useMemory(t1Allocator.allocate(1), t1Size);
    allocator<T1> t2Allocator = t1Allocator;
    useMemory(t2Allocator.allocate(1), t2Size);
}

struct Foo {
    const int x;
    const int y;

    Foo(int x, int y) : x(x), y(y) {
    }

    Foo() : x(0), y(0) {
    }
};

TEST(SingleThread, TemplateAllocateConstrArgs) {
    auto* pool = pool::create(sizeof(Foo), pool_type::SingleThreaded);
    {
        std::pmr::polymorphic_allocator<Foo> fooAllocator(pool);
        auto* foo = fooAllocator.new_object<Foo>(10, 20);
        EXPECT_EQ(10, foo->x);
        EXPECT_EQ(20, foo->y);
        useMemory(foo, sizeof(Foo));
        assertPoolFull(*pool);
    }
    delete pool;
}

template<typename T>
[[nodiscard]] constexpr size_t getAlignedSize() {
    const auto alignment = alignof(T);
    const auto size = sizeof(T);
    const auto extra = size % alignment;
    if (extra == 0)
        return size;
    return size + (alignment - extra);
}

TEST(SingleThread, TemplatePoolVectorConstrArgs) {
    constexpr auto count = 10;
    auto* pool = pool::create(getAlignedSize<Foo>() * count, pool_type::SingleThreaded);
    {
        std::pmr::vector<Foo> vec(pool);
        vec.reserve(count);
        for (int i = 0; i < count; ++i) {
            const auto expectedX = 10 * i;
            const auto expectedY = 20 * i;
            vec.emplace_back(expectedX, expectedY);
            EXPECT_EQ(expectedX, vec[i].x);
            EXPECT_EQ(expectedY, vec[i].y);
            useMemory(&vec[i], sizeof(Foo));
        }
    }
    delete pool;
}

TEST(SingleThread, TemplateAllocatorVectorConstrArgs2) {
    constexpr auto count = 10;

    allocator<Foo> a(getAlignedSize<Foo>() * count, pool_type::SingleThreaded);
    std::vector<Foo, allocator<Foo>> vec(a);
    vec.reserve(count);
    for (int i = 0; i < count; ++i) {
        const auto expectedX = 10 * i;
        const auto expectedY = 20 * i;
        vec.emplace_back(expectedX, expectedY);
        EXPECT_EQ(expectedX, vec[i].x);
        EXPECT_EQ(expectedY, vec[i].y);
        useMemory(&vec[i], sizeof(Foo));
    }
}

TEST(SingleThread, AssignVector) {
    allocator<int> a(123);
    std::vector<int, allocator<int>> v1(a);
    v1.push_back(1);
    const auto usage1 = a.get_pool()->get_size();

    std::vector<int, allocator<int>> v2 = v1;
    v2.push_back(2);
    const auto usage2 = a.get_pool()->get_size();
    EXPECT_GT(usage2, usage1);
}

TEST(SingleThread, ConstructFromAllocator) {
    allocator<Foo> a(1000);
    auto x = a.allocate_object(2, 3);
    EXPECT_EQ(2, x->x);
    EXPECT_EQ(3, x->y);
    useMemory(&x, sizeof(Foo));
}

TEST(SingleThread, ConstructMultipleTypesFromAllocator) {
    allocator<int> a(1000);
    auto i = a.allocate_object(123);
    EXPECT_EQ(123, *i);
    useMemory(i, sizeof(int));
    auto x = a.allocate_object<Foo>(2, 3);
    EXPECT_EQ(2, x->x);
    EXPECT_EQ(3, x->y);
    useMemory(&x, sizeof(Foo));
}

TEST(SingleThread, ConstructAllocatorNewType) {
    allocator<int> a(1000);
    std::vector<long, allocator<long>> v(a);
}
