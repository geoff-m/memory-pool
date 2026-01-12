#include "TestUtils.h"
#include <cstring>

void useMemory(void* buffer, size_t size) {
    EXPECT_NE(nullptr, buffer);
    std::memset(buffer, 0, size);
}

