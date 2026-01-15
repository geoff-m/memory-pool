#ifdef _WIN32
#include <stdexcept>
#include <system_error>
#include "internal.h"
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

using namespace memory_pool;

char* pool::allocate_buffer(const size_t size) {
    void* ret = VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (ret == nullptr) {
        throw std::system_error(GetLastError(), std::generic_category(),
                                "Failed to allocate memory");
    }
    return static_cast<char*>(ret);
}

void pool::free_buffer(char* buffer, const size_t size) {
    if (VirtualFree(buffer, 0, MEM_RELEASE) == 0) {
        throw std::system_error(GetLastError(), std::generic_category(),
                                "Failed to deallocate memory");
    }
}

#endif
