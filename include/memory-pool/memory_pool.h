#pragma once
#include <cstddef>
#include <memory_resource>
#include <utility>

namespace memory_pool {
    enum class pool_type {
        // Indicates that memory will be allocated from the pool in only one thread.
        SingleThreaded,

        // Makes it safe to allocate from the pool on multiple threads.
        ThreadSafe,

        // Makes the pool act as if it's a separate pool for each thread in the program.
        PerThread
    };

    class pool : public std::pmr::memory_resource {
    public:
        pool(const pool&) = delete;

        [[nodiscard]] static pool* create(size_t capacity);

        [[nodiscard]] static pool* create(size_t capacity, pool_type type);

        // Gets the maximum size in bytes of this pool.
        [[nodiscard]] virtual size_t get_capacity() const = 0;

        // Gets the number of bytes currently allocated in this pool.
        [[nodiscard]] virtual size_t get_size() const = 0;

        // Gets the number of bytes wasted due to alignment requests.
        [[nodiscard]] virtual size_t get_alignment_fragmentation() const = 0;

        void* do_allocate(std::size_t size, std::size_t alignment) override = 0;
        void* do_allocate(std::size_t size);

    protected:
        pool() = default;

        void do_deallocate(void* p, std::size_t size, std::size_t alignment) override;

        [[nodiscard]] bool do_is_equal(const memory_resource& other) const noexcept override;

        [[nodiscard]] static char* reserve_buffer(size_t size);

        static void allocate_reservation(char* buffer, size_t size);

        static void free_buffer(char* buffer, size_t size);

        [[nodiscard]] static size_t get_page_size();

        [[nodiscard]] static char* get_containing_page(char* pointer);
    };
}
