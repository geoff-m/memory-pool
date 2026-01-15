#ifdef __linux__
#include <stdexcept>
#include <system_error>
#include "internal.h"
#include <sys/mman.h>

using namespace memory_pool;

char* pool::allocate_buffer(const size_t size) {
    void* ret = mmap(nullptr,
                     size,
                     PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS,
                     -1,
                     0);
    if (ret == MAP_FAILED) {
        throw std::system_error(errno, std::generic_category(),
                                "Failed to allocate memory");
    }
    return static_cast<char*>(ret);
}

void pool::free_buffer(char* buffer, const size_t size) {
    if (munmap(buffer, size) == -1) {
        throw std::system_error(errno, std::generic_category(),
                                "Failed to deallocate memory");
    }
}

#endif
