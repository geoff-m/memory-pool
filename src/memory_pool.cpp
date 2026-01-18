#include "memory-pool/memory_pool.h"
#include "internal.h"
#include <stdexcept>
#include <unordered_map>
#include <memory>
#include <string>
#include <bit>
#include <cassert>
#include <sstream>
#include <iomanip>

using namespace memory_pool;

pool* pool::create(const size_t capacity) {
    return create(capacity, pool_type::ThreadSafe);
}

pool* pool::create(const size_t capacity, const pool_type type) {
    switch (type) {
        case pool_type::SingleThreaded:
            return new simple_pool(capacity);
        case pool_type::PerThread:
            return new pool_per_thread(capacity);
        default:
        case pool_type::ThreadSafe:
            return new locked_pool(capacity);
    }
}

void* pool::do_allocate(const std::size_t size) {
    return do_allocate(size, 1);
}

void pool::do_deallocate(void* p, std::size_t bytes, std::size_t alignment) {
    // Do nothing.
}

bool pool::do_is_equal(const memory_resource& other) const noexcept {
    return this == &other;
}

size_t roundUpToPowerOf2(const size_t n) {
    if (n == 0)
        return 1;
    if ((n & (n - 1)) == 0)
        return n; // Already a power of 2.
    return static_cast<size_t>(1) << (64 - std::countl_zero(n));
}

size_t computeCommitAheadBytes(size_t pageSize) {
    constexpr auto oneMegabyte = static_cast<size_t>(1 << 20);
    if (pageSize < oneMegabyte) {
        return oneMegabyte;
    }
    return roundUpToPowerOf2(pageSize);
}

simple_pool::simple_pool(const size_t capacity)
    : totalCapacity(capacity),
      commitAheadBytes(computeCommitAheadBytes(get_page_size())) {
    buffer = reserve_buffer(capacity);
    bytesInUse = 0;
    firstCommittedUnusedByte = buffer;
    const auto initialCommit = std::min(capacity, commitAheadBytes);
    allocate_reservation(firstCommittedUnusedByte, initialCommit);
    firstUncommittedByte = buffer + initialCommit;
}

simple_pool::~simple_pool() {
    free_buffer(buffer, totalCapacity);
}

size_t simple_pool::get_alignment_fragmentation() const {
    return alignmentFragmentationBytes;
}

void simple_pool::printStats() {
    std::stringstream ss;
    ss << std::hex;
    ss << "Pool " << reinterpret_cast<uintptr_t>(this) << ":\n";
    ss << "  Uncommitted reserved: " << (buffer + totalCapacity) - firstUncommittedByte << '\n';
    ss << "  Committed unused: " << firstUncommittedByte - firstCommittedUnusedByte << '\n';
    ss << "  Used: " << firstCommittedUnusedByte - buffer << '\n';
    printf("%s\n", ss.str().c_str());
    fflush(stdout);
}

size_t simple_pool::get_size() const {
    return bytesInUse;
}

size_t simple_pool::get_capacity() const {
    return totalCapacity;
}

[[nodiscard]] size_t computeAlignmentSkip(const char* pointer, const size_t alignment) {
    size_t remainder;
    if ((alignment & (alignment - 1)) == 0) {
        // Alignment is a power of 2.
        remainder = reinterpret_cast<uintptr_t>(pointer) & (alignment - 1);
    } else {
        remainder = reinterpret_cast<uintptr_t>(pointer) % alignment;
    }
    if (remainder == 0)
        return 0;
    return alignment - remainder;
}

void* simple_pool::do_allocate(std::size_t size, std::size_t alignment) {
    if (totalCapacity - bytesInUse < size) [[unlikely]] {
        std::string message = "Out of memory: ";
        message += std::to_string(size) + " bytes requested, but pool has ";
        message += std::to_string(totalCapacity - bytesInUse);
        message += " bytes free";
        throw std::invalid_argument(message);
    }

    const auto alignmentSkip = computeAlignmentSkip(firstCommittedUnusedByte, alignment);

    const size_t toCommitAhead = (alignmentSkip + size * 2 + (commitAheadBytes - 1)) & ~(commitAheadBytes - 1);

    if (firstCommittedUnusedByte + toCommitAhead > firstUncommittedByte) {
        size_t toCommit;
        if (firstUncommittedByte - buffer + toCommitAhead > totalCapacity) {
            toCommit = (buffer + totalCapacity) - firstUncommittedByte;
        } else {
            toCommit = toCommitAhead;
        }
        if (toCommit > 0) {
            auto* firstUncommittedPage = get_containing_page(firstUncommittedByte);
            const auto pointInPage = firstUncommittedByte - firstUncommittedPage;
            allocate_reservation(firstUncommittedPage, toCommit);
            firstUncommittedByte += toCommit;
        }
    }

    void* ret = alignmentSkip + firstCommittedUnusedByte;
    auto* newFirstCommittedUnusedByte = firstCommittedUnusedByte + alignmentSkip + size;
    if (newFirstCommittedUnusedByte > buffer + totalCapacity) {
        std::string message = "Out of memory: ";
        message += std::to_string(size) + " bytes requested with ";
        message += std::to_string(alignment) + "-byte alignment, which pool cannot fit in its last ";
        message += std::to_string(totalCapacity - bytesInUse);
        message += " free bytes";
        throw std::invalid_argument(message);
    }

    firstCommittedUnusedByte = newFirstCommittedUnusedByte;
    bytesInUse += alignmentSkip + size;
    alignmentFragmentationBytes += alignmentSkip;

    if (bytesInUse == totalCapacity) {
        assert(firstUncommittedByte == buffer + totalCapacity);
        assert(firstCommittedUnusedByte == buffer + totalCapacity);
    } else {
        assert(firstCommittedUnusedByte < firstUncommittedByte);
    }

    return ret;
}

locked_pool::locked_pool(const size_t capacity)
    : pool(capacity) {
}

void* locked_pool::do_allocate(std::size_t size, std::size_t alignment) {
    std::lock_guard lock(mutex);
    return pool.do_allocate(size, alignment);
}

size_t locked_pool::get_alignment_fragmentation() const {
    std::lock_guard lock(mutex);
    return pool.get_alignment_fragmentation();
}

size_t locked_pool::get_capacity() const {
    std::lock_guard lock(mutex);
    return pool.get_capacity();
}

size_t locked_pool::get_size() const {
    return pool.get_size();
}

pool_per_thread::pool_per_thread(const size_t capacity)
    : totalCapacity(capacity) {
}


size_t pool_per_thread::get_capacity() const {
    return get_thread_local_pool()->get_capacity();
}

size_t pool_per_thread::get_size() const {
    return get_thread_local_pool()->get_size();
}

size_t pool_per_thread::get_alignment_fragmentation() const {
    return get_thread_local_pool()->get_alignment_fragmentation();
}

void* pool_per_thread::do_allocate(std::size_t size, std::size_t alignment) {
    return get_thread_local_pool()->allocate(size, alignment);
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
    return new simple_pool(totalCapacity);
}
