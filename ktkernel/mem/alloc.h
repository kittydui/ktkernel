#pragma once
#include <cstddef>

namespace kt_kernel
{
    void* kmalloc(size_t size);
    void kfree(void* ptr);
} // namespace kt_kernel
