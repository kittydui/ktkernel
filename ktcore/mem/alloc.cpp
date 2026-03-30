#include "mem/alloc.h"
#include "systems.h"

namespace KtCore
{
void* KMalloc(size_t size)
{
    return g_kernelContext->m_allocator->allocate(size);
}

void KFree(void* ptr)
{
    g_kernelContext->m_allocator->free(ptr);
}
} // namespace KtCore
