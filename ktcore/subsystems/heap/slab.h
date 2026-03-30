#pragma once
#include <cstddef>
#include <cstdint>

namespace KtCore
{
class SlabAllocator
{
public:
    void initialize();
    void* allocate(size_t size);
    void free(void* ptr);

private:
    static constexpr uint64_t MAGIC_NUMBER = 0x534C4142;
    static constexpr uint64_t PAGE_SIZE = 4096;
    static constexpr size_t CLASS_SZ[6] = { 32, 64, 128, 256, 512, 1024 };

    struct SlabHeader
    {
        uint64_t m_magicNumber;
        void* m_freeList;
        SlabHeader* m_next;
        size_t m_objectSize;
        size_t m_usedCount;
        size_t m_totalCount;
    };

    struct LargeHeader
    {
        size_t m_pages;
        size_t padding_0;
    };

    struct Cache
    {
        SlabHeader* slabs = nullptr;
        size_t objectSize = 0;
    };

    Cache m_caches[6];
    uint64_t m_hhdmOffset = 0;

    int findClass(size_t size);
    SlabHeader* createSlab(size_t size);
    void* allocateFromSlab(Cache& cache);
    void* allocateLarge(size_t size);
};
} // namespace KtCore
