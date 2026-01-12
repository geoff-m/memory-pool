#include <gtest/gtest.h>
#include "memory_pool.h"
#include <vector>
#include "TestUtils.h"

TEST(CustomAllocator, CreateVector) {
    memory_pool<int> pool(10000);
    std::vector<int, memory_pool<int>> v(pool);
}

TEST(CustomAllocator, UseVector) {
    using TItem = int;
    memory_pool<TItem> pool(100);
    std::vector<TItem, memory_pool<TItem>> v(pool);
    v.emplace_back(1);
    v.emplace_back(2);
    v.emplace_back(3);
}