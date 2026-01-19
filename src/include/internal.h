#pragma once
#include "memory-pool/memory_pool.h"
#include <mutex>

using namespace memory_pool;

class simple_pool : public pool {
    const size_t totalCapacity;
    size_t commitAheadBytes;
    size_t bytesInUse = 0;
    char* buffer; // Page-aligned.
    char* firstCommittedUnusedByte;
    char* firstUncommittedByte; // Page-aligned.
    size_t alignmentFragmentationBytes = 0;
    // low address ---uuuuuuuuuuuuuuuuuuuuuuccccccccccccccccccccccrrrrrrrrrrrrrrrrr----- high address
    //                ^                     ^                     ^
    //                buffer               firstCommittedUnused   firstUncommitted
    // u = in use
    // c = committed (not in use)
    // r = reserved (not in use, not committed)

public:
    explicit simple_pool(size_t capacity);

    ~simple_pool() override;

    [[nodiscard]] size_t get_alignment_fragmentation() const override;

    [[nodiscard]] size_t get_size() const override;

    [[nodiscard]] size_t get_capacity() const override;

    void* do_allocate(std::size_t bytes, std::size_t alignment) override;

private:
    void printStats();
};

class locked_pool : public pool {
    simple_pool pool;
    mutable std::mutex mutex;

    void* do_allocate(std::size_t size, std::size_t alignment) override;

public:

    [[nodiscard]] size_t get_alignment_fragmentation() const override;

    explicit locked_pool(size_t capacity);

    [[nodiscard]] size_t get_capacity() const override;

    [[nodiscard]] size_t get_size() const override;
};

class pool_per_thread : public pool {
public:
    explicit pool_per_thread(size_t capacity);

    [[nodiscard]] size_t get_capacity() const override;

    [[nodiscard]] size_t get_size() const override;

    [[nodiscard]] size_t get_alignment_fragmentation() const override;

private:
    void* do_allocate(std::size_t size, std::size_t alignment) override;

    [[nodiscard]] pool* get_thread_local_pool() const;

    [[nodiscard]] pool* create_pool() const;

    const size_t totalCapacity;
};
