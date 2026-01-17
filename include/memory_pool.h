#pragma once
#include <cstddef>
#include <utility>

namespace memory_pool {
    enum class out_of_memory_behavior {
        Throw,
        ReturnNull
    };

    enum class pool_type {
        // Indicates that memory will be allocated from the pool in only one thread.
        SingleThreaded,

        // Makes it safe to allocate from the pool on multiple threads.
        ThreadSafe,

        // Makes the pool act as if it's a separate pool for each thread in the program.
        PerThread
    };

    class pool {
    public:
        virtual ~pool() = default;

        pool(const pool&) = delete;

        [[nodiscard]] static pool* create(size_t capacity);

        [[nodiscard]] static pool* create(size_t capacity,
                                          pool_type type,
                                          out_of_memory_behavior oomBehavior);

        [[nodiscard]] virtual void* allocate(size_t size) = 0;

        template<typename T, typename... Args>
        [[nodiscard]] T* allocate(Args&&... args) {
            return new(static_cast<T*>(allocate(sizeof(T)))) T(std::forward<Args>(args)...);
        }

        // Gets the maximum size in bytes of this pool.
        [[nodiscard]] virtual size_t get_capacity() const = 0;

        // Gets the number of bytes currently allocated in this pool.
        [[nodiscard]] virtual size_t get_size() const = 0;

    protected:
        [[nodiscard]] static char* reserve_buffer(size_t size);

        static void allocate_reservation(char* buffer, size_t size);

        static void free_buffer(char* buffer, size_t size);

        [[nodiscard]] static size_t get_page_size();

        [[nodiscard]] static char* get_containing_page(char* pointer);

        pool() = default;
    };
}
