#pragma once
#include "memory-pool/memory_pool.h"
#include <mutex>

using namespace memory_pool;
class simple_pool : public pool {
    const size_t totalCapacity;
    size_t commitAheadBytes;
    size_t bytesInUse;
    char* buffer; // Page-aligned.
    char* firstCommittedUnusedByte;
    char* firstUncommittedByte; // Page-aligned.
    // low address ---uuuuuuuuuuuuuuuuuuuuuuccccccccccccccccccccccrrrrrrrrrrrrrrrrr----- high address
    //                ^                     ^                     ^
    //                buffer               firstCommittedUnused   firstUncommitted
    // u = in use
    // c = committed (not in use)
    // r = reserved (not in use, not committed)
    const out_of_memory_behavior oomBehavior;

public:
    simple_pool(size_t capacity, out_of_memory_behavior oomBehavior);

    ~simple_pool() override;

    [[nodiscard]] void* allocate(size_t size) override;

    [[nodiscard]] size_t get_size() const override;

    [[nodiscard]] size_t get_capacity() const override;

private:
    void printStats();
};

class locked_pool : public pool {
    simple_pool pool;
    mutable std::mutex mutex;

public:
    locked_pool(size_t capacity, out_of_memory_behavior oomBehavior);

    [[nodiscard]] void* allocate(size_t size) override;

    [[nodiscard]] size_t get_capacity() const override;

    [[nodiscard]] size_t get_size() const override;
};

class pool_per_thread : public pool {
public:
    pool_per_thread(size_t capacity, out_of_memory_behavior oomBehavior);

    [[nodiscard]] void* allocate(size_t size) override;

    [[nodiscard]] size_t get_capacity() const override;

    [[nodiscard]] size_t get_size() const override;

private:
    [[nodiscard]] pool* get_thread_local_pool() const;

    [[nodiscard]] pool* create_pool() const;

    const size_t totalCapacity;
    const out_of_memory_behavior oomBehavior;
};