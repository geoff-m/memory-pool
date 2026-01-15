#pragma once
#include "memory_pool.h"

void usePool(memory_pool::pool& pool, size_t chunkSize);

void useMemory(void* buffer, size_t size);

void assertPoolFull(memory_pool::pool& pool);