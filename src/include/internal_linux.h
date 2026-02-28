#pragma once
#include <pthread.h>

namespace memory_pool {
    using NativeThreadHandle = pthread_t;
    NativeThreadHandle getThreadHandle();
}
