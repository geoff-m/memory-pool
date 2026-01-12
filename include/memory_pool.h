#pragma once
#include <cstddef>
//#include "internal.h"
#include <string>
#include <stdexcept>

enum class out_of_memory_behavior {
    Throw,
    ReturnNull
};

[[nodiscard]] char* allocate_buffer(size_t size);
void free_buffer(char* buffer, size_t size);

template<typename T>
class memory_pool {
    const size_t totalCapacity;
    size_t remainingCapacity;
    char* buffer;
    char* firstUnusedByte;
    out_of_memory_behavior oomBehavior;

public:
    using value_type = T;

    explicit memory_pool(size_t capacity)
        : memory_pool(capacity,
                      true, false, out_of_memory_behavior::Throw) {
    }

    memory_pool(size_t capacity,
                bool threadSafe,
                bool poolPerThread,
                out_of_memory_behavior oomBehavior) : totalCapacity(capacity),
                                                      remainingCapacity(capacity),
                                                      oomBehavior(oomBehavior) {
        buffer = allocate_buffer(capacity);
        firstUnusedByte = buffer;
    }

    ~memory_pool() {
        free_buffer(buffer, totalCapacity);
    }

    memory_pool(const memory_pool& other) = default;

    //[[nodiscard]] void* allocate(size_t size);

    [[nodiscard]] T* allocate(std::size_t count) {
        return static_cast<T*>(allocate_region(count * sizeof(T)));
    }

    void deallocate(T* p, std::size_t count) noexcept {
        // Do nothing. Deallocation not supported.
    }

    [[nodiscard]] size_t get_capacity() const {
        return totalCapacity;
    }

    [[nodiscard]] size_t get_size() const {
        return totalCapacity - remainingCapacity;
    }

private:
    void* allocate_region(size_t size) {
        if (remainingCapacity < size) [[unlikely]] {
            if (oomBehavior == out_of_memory_behavior::Throw) {
                std::string message = "Out of memory: ";
                message += std::to_string(size) + " bytes requested, but pool only has ";
                message += std::to_string(remainingCapacity);
                message += " bytes free";
                throw std::invalid_argument(message);
            } else {
                return nullptr;
            }
        }

        void* ret = firstUnusedByte;
        firstUnusedByte += size;
        remainingCapacity -= size;
        return ret;
    }
};