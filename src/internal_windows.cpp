#ifdef _WIN32
#include <stdexcept>
#include <system_error>
#include "internal.h"
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

using namespace memory_pool;

size_t getPageSize() {
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return info.dwPageSize;
}

size_t pool::get_page_size() {
	return getPageSize();
}

char* pool::get_containing_page(char* pointer) {
	static const uintptr_t pageMaskOn = ~(get_page_size() - 1);
	return reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(pointer) & pageMaskOn);
}

char* pool::reserve_buffer(const size_t size) {
	void* ret = VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_READWRITE);
	if (ret == nullptr) {
		throw std::system_error(GetLastError(), std::generic_category(),
			"Failed to allocate memory");
	}
	return static_cast<char*>(ret);
}

void pool::allocate_reservation(char* buffer, const size_t size) {
	if (VirtualAlloc(buffer, size, MEM_COMMIT, PAGE_READWRITE) == 0) {
		throw std::system_error(errno, std::generic_category(),
			"Failed to allocate memory");
	}
}

void pool::free_buffer(char* buffer, const size_t size) {
	if (VirtualFree(buffer, 0, MEM_RELEASE) == 0) {
		throw std::system_error(GetLastError(), std::generic_category(),
			"Failed to deallocate memory");
	}
}

#endif
