#include "subsystems/heap/slab.h"
#include "limine/requests.h"
#include "mem/memory.h"
#include "systems.h"

namespace KtCore
{
void SlabAllocator::initialize()
{
    m_hhdmOffset = g_hhdmRequest.response->offset;

    for (size_t i = 0; i < 6; i++) {
        m_caches[i].objectSize = CLASS_SZ[i];
        m_caches[i].slabs = nullptr;
    }
}

int SlabAllocator::findClass(size_t size)
{
    for (size_t i = 0; i < 6; i++)
        if (size <= CLASS_SZ[i])
            return static_cast<int>(i);

    return -1;
}

SlabAllocator::SlabHeader* SlabAllocator::createSlab(size_t size)
{
    uint64_t frame = g_kernelContext->m_pmm->allocateFrame();
    if (!frame)
        return nullptr;

    auto* slab = reinterpret_cast<SlabHeader*>(frame + m_hhdmOffset);
    slab->m_magicNumber = MAGIC_NUMBER;
    slab->m_objectSize = size;
    slab->m_usedCount = 0;
    slab->m_next = nullptr;
    slab->m_freeList = nullptr;

    auto data_start = reinterpret_cast<uint64_t>(slab) + sizeof(SlabHeader);
    size_t available = PAGE_SIZE - sizeof(SlabHeader);
    slab->m_totalCount = available / size;

    for (size_t i = 0; i < slab->m_totalCount; i++) {
        auto* object = reinterpret_cast<void**>(data_start + i * size);
        *object = slab->m_freeList;
        slab->m_freeList = object;
    }

    return slab;
}

void* SlabAllocator::allocateFromSlab(Cache& cache)
{
    SlabHeader* slab = cache.slabs;
    while (slab && !slab->m_freeList)
        slab = slab->m_next;

    if (!slab) {
        slab = createSlab(cache.objectSize);
        if (!slab)
            return nullptr;

        slab->m_next = cache.slabs;
        cache.slabs = slab;
    }

    void* object = slab->m_freeList;
    slab->m_freeList = *reinterpret_cast<void**>(object);
    slab->m_usedCount++;
    return object;
}

void* SlabAllocator::allocateLarge(size_t size)
{
    size_t total = size + sizeof(LargeHeader);
    size_t pages = (total + PAGE_SIZE - 1) / PAGE_SIZE;

    uint64_t frame = g_kernelContext->m_pmm->allocateContiguous(pages);
    if (!frame)
        return nullptr;

    auto* header = reinterpret_cast<LargeHeader*>(frame + m_hhdmOffset);
    header->m_pages = pages;

    return header + 1;
}

void* SlabAllocator::allocate(size_t size)
{
    if (size == 0)
        return nullptr;

    int idx = findClass(size);
    if (idx >= 0)
        return allocateFromSlab(m_caches[idx]);

    return allocateLarge(size);
}

void SlabAllocator::free(void* ptr)
{
    if (!ptr)
        return;

    auto page = reinterpret_cast<uint64_t>(ptr) & ~(PAGE_SIZE - 1);
    auto* slab = reinterpret_cast<SlabHeader*>(page);

    if (slab->m_magicNumber == MAGIC_NUMBER) {
        *reinterpret_cast<void**>(ptr) = slab->m_freeList;
        slab->m_freeList = ptr;
        slab->m_usedCount--;
        return;
    }

    auto* header = reinterpret_cast<LargeHeader*>(ptr) - 1;
    uint64_t phys = reinterpret_cast<uint64_t>(header) - m_hhdmOffset;
    g_kernelContext->m_pmm->freeContiguous(phys, header->m_pages);
}
} // namespace KtCore
