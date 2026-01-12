#pragma once
#include <cstddef>

enum class out_of_memory_behavior {
    Throw,
    ReturnNull
};

class memory_pool {
    const size_t totalCapacity;
    size_t remainingCapacity;
    char* buffer;
    char* firstUnusedByte;
    out_of_memory_behavior oomBehavior;
    [[nodiscard]] static char* allocate_buffer(size_t size);
    static void free_buffer(char* buffer, size_t size);

public:
    explicit memory_pool(size_t capacity);
    memory_pool(size_t capacity,
        bool threadSafe,
        bool poolPerThread,
        out_of_memory_behavior oomBehavior);
    ~memory_pool();
    memory_pool(const memory_pool&) = delete;

    [[nodiscard]] void* allocate(size_t size);

    [[nodiscard]] size_t get_capacity() const;
    [[nodiscard]] size_t get_size() const;
};
