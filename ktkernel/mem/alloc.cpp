#include "mem/alloc.h"
#include "symbol.h"
#include "systems.h"

namespace kt_kernel
{
    void* kmalloc(size_t size)
    {
        return g_kernel_context->allocator->allocate(size);
    }

    void kfree(void* ptr)
    {
        g_kernel_context->allocator->free(ptr);
    }

    kt_export_symbol(kmalloc);
    kt_export_symbol(kfree);
} // namespace kt_kernel
