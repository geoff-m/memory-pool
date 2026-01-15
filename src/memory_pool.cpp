#include "memory_pool.h"
#include "internal.h"
#include <stdexcept>
#include <unordered_map>
#include <memory>
#include <string>

using namespace memory_pool;

pool* pool::create(const size_t capacity) {
    return create(capacity, pool_type::ThreadSafe, out_of_memory_behavior::Throw);
}

pool* pool::create(const size_t capacity, const pool_type type,
                                 const out_of_memory_behavior oomBehavior) {
    switch (type) {
        case pool_type::SingleThreaded:
            return new simple_pool(capacity, oomBehavior);
        case pool_type::PerThread:
            return new pool_per_thread(capacity, oomBehavior);
        default:
        case pool_type::ThreadSafe:
            return new locked_pool(capacity, oomBehavior);
    }
}

simple_pool::simple_pool(const size_t capacity, const out_of_memory_behavior oomBehavior)
    : totalCapacity(capacity),
      remainingCapacity(capacity),
      oomBehavior(oomBehavior) {
    buffer = allocate_buffer(capacity);
    firstUnusedByte = buffer;
}

simple_pool::~simple_pool() {
    free_buffer(buffer, totalCapacity);
}

void* simple_pool::allocate(const size_t size) {
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

size_t simple_pool::get_size() const {
    return totalCapacity - remainingCapacity;
}

size_t simple_pool::get_capacity() const {
    return totalCapacity;
}

locked_pool::locked_pool(const size_t capacity, const out_of_memory_behavior oomBehavior)
    : pool(capacity, oomBehavior) {
}

void* locked_pool::allocate(const size_t size) {
    std::lock_guard lock(mutex);
    return pool.allocate(size);
}

size_t locked_pool::get_capacity() const {
    std::lock_guard lock(mutex);
    return pool.get_capacity();
}

size_t locked_pool::get_size() const {
    return pool.get_size();
}

pool_per_thread::pool_per_thread(const size_t capacity, const out_of_memory_behavior oomBehavior)
    : totalCapacity(capacity),
      oomBehavior(oomBehavior) {
}

void* pool_per_thread::allocate(const size_t size) {
    return get_thread_local_pool()->allocate(size);
}

size_t pool_per_thread::get_capacity() const {
    return get_thread_local_pool()->get_capacity();
}

size_t pool_per_thread::get_size() const {
    return get_thread_local_pool()->get_size();
}

pool* pool_per_thread::get_thread_local_pool() const {
    static thread_local std::unordered_map<const pool_per_thread*, std::unique_ptr<pool>> threadLocalPools;
    const auto it = threadLocalPools.find(this);
    if (it != threadLocalPools.end()) {
        return it->second.get();
    }
    auto* ret = create_pool();
    threadLocalPools[this] = std::unique_ptr<pool>(ret);
    return ret;
}

pool* pool_per_thread::create_pool() const {
    return new simple_pool(totalCapacity, oomBehavior);
}