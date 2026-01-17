#include "TestUtils.h"
#include <gtest/gtest.h>
#include <cstring>

using namespace memory_pool;
void usePool(pool& pool, const size_t chunkSize) {
    while (pool.get_size() < pool.get_capacity())
        (void)pool.allocate(chunkSize);
}

void useMemory(void* buffer, const size_t size) {
    EXPECT_NE(nullptr, buffer);
    std::memset(buffer, 0, size);
}

void assertPoolFull(pool& pool) {
    EXPECT_EQ(pool.get_size(), pool.get_capacity());
    try {
        EXPECT_EQ(nullptr, pool.allocate(1));
    } catch (...) {
        return;
    }
}
