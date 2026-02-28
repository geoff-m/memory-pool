#pragma once
#include <Windows.h>

namespace memory_pool {
    using NativeThreadHandle = HANDLE;
    HANDLE getThreadHandle();
}
