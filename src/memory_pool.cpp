#include "memory_pool.h"
#include <stdexcept>

memory_pool::memory_pool(size_t capacity)
    : memory_pool(capacity,
                  true,
                  false,
                  out_of_memory_behavior::Throw) {
}

memory_pool::memory_pool(const size_t capacity,
                         const bool threadSafe,
                         const bool poolPerThread,
                         const out_of_memory_behavior oomBehavior)
    : totalCapacity(capacity),
      remainingCapacity(capacity),
      oomBehavior(oomBehavior) {
    buffer = allocate_buffer(capacity);
    firstUnusedByte = buffer;
}

memory_pool::~memory_pool() {
    free_buffer(buffer, totalCapacity);
}

void* memory_pool::allocate(const size_t size) {
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

size_t memory_pool::get_capacity() const {
    return totalCapacity;
}

size_t memory_pool::get_size() const {
    return totalCapacity - remainingCapacity;
}
