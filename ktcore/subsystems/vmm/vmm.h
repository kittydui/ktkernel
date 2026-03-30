#pragma once
#include <cstddef>
#include <cstdint>

namespace KtCore
{
    enum PageFlags : uint64_t
    {
        PRESENT = 1ULL << 0,
        WRITABLE = 1ULL << 1,
        USER = 1ULL << 2,
        WRITE_THROUGH = 1ULL << 3,
        CACHE_DISABLE = 1ULL << 4,
        ACCESSED = 1ULL << 5,
        DIRTY = 1ULL << 6,
        HUGE_PAGE = 1ULL << 7,
        GLOBAL = 1ULL << 8,
        NX = 1ULL << 63,
        ADDRESS_MASK = 0x000FFFFFFFFFF000ULL,
    };

    class VMM
    {
    public:
        void initialize();

        bool map(uint64_t virt, uint64_t phys, uint64_t flags);
        void unMap(uint64_t virt);

        uint64_t virtualToPhysical(uint64_t virt);
        uint64_t physicalToVirtual(uint64_t phys);

        void activate();

    private:
        uint64_t m_pml4Physical = 0;
        uint64_t m_hhdmOffset = 0;
        uint64_t* getPte(uint64_t virt, bool create);
    };
} // namespace KtCore
