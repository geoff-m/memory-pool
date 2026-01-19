#pragma once
#include <cstddef>
#include <cstdio>
#include <memory_resource>
#include <utility>
#include <memory>

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

        // Allocates a region of memory with the given size and alignment.
        [[nodiscard]] void* new_buffer(std::size_t size, std::size_t alignment);

        // Allocates a region of memory (unaligned).
        [[nodiscard]] void* new_buffer(std::size_t size);

        // Allocates and constructs a new object.
        template<typename T, typename... Args>
        [[nodiscard]] T* new_object(Args&&... args) {
            return new(allocate(sizeof(T), alignof(T))) T(std::forward<Args>(args)...);
        }

    protected:
        pool() = default;

        [[nodiscard]] void* do_allocate(std::size_t size, std::size_t alignment) override = 0;

        [[nodiscard]] void* do_allocate(std::size_t size);

        void do_deallocate(void* p, std::size_t size, std::size_t alignment) override;

        [[nodiscard]] bool do_is_equal(const memory_resource& other) const noexcept override;

        [[nodiscard]] static char* reserve_buffer(size_t size);

        static void allocate_reservation(char* buffer, size_t size);

        static void free_buffer(char* buffer, size_t size);

        [[nodiscard]] static size_t get_page_size();

        [[nodiscard]] static char* get_containing_page(char* pointer);
    };

    template<class T>
    class allocator {
        pool* impl;

    public:
        using value_type = T;

        explicit allocator(pool* pool)
            : impl(pool) {
        }

        template<class U>
        allocator(allocator<U> const& other) noexcept
            : impl(other.get_pool()) {
        }

        template<class U>
        allocator(allocator<U>&& other) noexcept
            : impl(other.get_pool()) {
        }


        [[nodiscard]] value_type* allocate(size_t count) {
            return static_cast<value_type*>(impl->allocate(count * sizeof(value_type), alignof(value_type)));
        }

        void deallocate(value_type*, size_t) noexcept {
            // Do nothing.
        }

        template<typename... Args>
        [[nodiscard]] value_type* allocate_object(Args&&... args) {
            return new(allocate(sizeof(value_type))) value_type(std::forward<Args>(args)...);
        }

        template<typename U, typename... Args>
        [[nodiscard]] U* allocate_object(Args&&... args) {
            return new(allocate(sizeof(U))) U(std::forward<Args>(args)...);
        }

        using propagate_on_container_copy_assignment = std::true_type;
        using propagate_on_container_move_assignment = std::true_type;
        using propagate_on_container_swap = std::true_type;

        [[nodiscard]] pool* get_pool() const {
            return impl;
        }

        template<class U>
        bool operator==(allocator<U> const& other) noexcept {
            return impl == other.impl;
        }

        template<class U>
        bool operator!=(allocator<U> const& other) noexcept {
            return !(*this == other);
        }
    };
}
