#ifdef __linux__
#include <stdexcept>
#include <system_error>
#include <unistd.h>

#include "internal.h"
#include <sys/mman.h>

using namespace memory_pool;

char* pool::reserve_buffer(const size_t size) {
    auto* ret = static_cast<char*>(mmap(nullptr,
                                        size,
                                        PROT_NONE,
                                        MAP_PRIVATE | MAP_ANONYMOUS,
                                        -1,
                                        0));
    if (ret == MAP_FAILED) {
        throw std::system_error(errno, std::generic_category(),
                                "Failed to allocate memory");
    }
    return ret;
}

size_t getPageSize() {
    return sysconf(_SC_PAGESIZE);
}

size_t pool::get_page_size() {
    return getPageSize();
}

char* pool::get_containing_page(char* pointer) {
    static const uintptr_t pageMaskOn = ~(get_page_size() - 1);
    return reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(pointer) & pageMaskOn);
 }


void pool::allocate_reservation(char* buffer, const size_t size) {
    if (mprotect(buffer, size, PROT_READ | PROT_WRITE) == -1) {
        throw std::system_error(errno, std::generic_category(),
                                "Failed to allocate memory");
    }
}

void pool::free_buffer(char* buffer, const size_t size) {
    if (munmap(buffer, size) == -1) {
        throw std::system_error(errno, std::generic_category(),
                                "Failed to deallocate memory");
    }
}

#endif
