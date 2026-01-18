#include "gtest/gtest.h"
#include "memory-pool/memory_pool.h"
#include "TestUtils.h"

using namespace memory_pool;

TEST(Alignment, Align4) {
    auto* pool = pool::create(100);
    constexpr auto expectedAlignment = 4;
    for (int i = 0; i < 8; ++i) {
        (void)pool->do_allocate(1, 1); // Perturb alignment
        const auto ptr = reinterpret_cast<uintptr_t>(pool->do_allocate(1, expectedAlignment));
        EXPECT_EQ(0, ptr % expectedAlignment);
    }
    delete pool;
}
