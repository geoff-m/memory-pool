#pragma once
#include <gtest/gtest.h>
#include "memory_pool.h"

template<typename T>
void usePool(memory_pool<T>& pool, size_t chunkSize) {
    while (pool.get_size() < pool.get_capacity())
        (void)pool.allocate(chunkSize);
}

void useMemory(void* buffer, size_t size);

template<typename T>
void assertPoolFull(memory_pool<T>& pool) {
    EXPECT_TRUE(pool.get_size() == pool.get_capacity());
    try {
        EXPECT_EQ(nullptr, pool.allocate(1));
    } catch (...) {
        return;
    }
}