#pragma once
#include <cstddef>
#include <utility>


namespace memory_pool {
    enum class out_of_memory_behavior {
        Throw,
        ReturnNull
    };

    enum class pool_type {
        SingleThreaded,
        ThreadSafe,
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

        [[nodiscard]] virtual size_t get_capacity() const = 0;

        [[nodiscard]] virtual size_t get_size() const = 0;

    protected:
        [[nodiscard]] static char* allocate_buffer(size_t size);

        static void free_buffer(char* buffer, size_t size);

        pool() = default;


    };
}