#include "gtest/gtest.h"
#include "memory-pool/memory_pool.h"
#include "../../src/include/internal.h"

[[nodiscard]] bool isPowerOf2(size_t value) {
    return std::has_single_bit(value);
}

TEST(RoundUpPowerOf2, Zero) {
    EXPECT_EQ(1, memory_pool::roundUpToPowerOf2(0));
}

TEST(RoundUpPowerOf2, One) {
    EXPECT_EQ(1, memory_pool::roundUpToPowerOf2(1));
}

TEST(RoundUpPowerOf2, AllPowersOf2) {
    for (size_t i = 1; i != 0; i <<= 1)
        EXPECT_EQ(i, memory_pool::roundUpToPowerOf2(i));
}

TEST(RoundUpPowersOf2, PowersOf2Plus1) {
    for (size_t i = 1; i != 0; i <<= 1) {
        const auto v = i + 1;
        size_t actual;
        if ((i << 1) < i) {
            try {
                actual = memory_pool::roundUpToPowerOf2(v);
            } catch (...) {
             continue;
            }
            FAIL() << "Expected exception for this value";
        } else {
            actual = memory_pool::roundUpToPowerOf2(v);
        }
        EXPECT_GE(actual, v);
        EXPECT_TRUE(isPowerOf2(actual));
    }
}

TEST(RoundUpPowersOf2, PowersOf2Minus1) {
    for (size_t i = 1; i != 0; i <<= 1) {
        const auto v = i - 1;
        const auto actual = memory_pool::roundUpToPowerOf2(v);
        EXPECT_GE(actual, v);
        EXPECT_TRUE(isPowerOf2(actual));
    }
}