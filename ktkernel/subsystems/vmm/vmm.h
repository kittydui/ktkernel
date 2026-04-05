#pragma once
#include <cstddef>
#include <cstdint>

namespace kt_kernel
{
    enum class page_flags : uint64_t
    {
        present = 1ULL << 0,
        writable = 1ULL << 1,
        user = 1ULL << 2,
        write_through = 1ULL << 3,
        cache_disable = 1ULL << 4,
        accessed = 1ULL << 5,
        dirty = 1ULL << 6,
        huge_page = 1ULL << 7,
        global = 1ULL << 8,
        nx = 1ULL << 63,
        address_mask = 0x000FFFFFFFFFF000ULL,
    };

    inline page_flags operator|(page_flags a, page_flags b)
    {
        return static_cast<page_flags>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b));
    }

    inline uint64_t operator&(uint64_t a, page_flags b)
    {
        return a & static_cast<uint64_t>(b);
    }

    inline uint64_t operator|(uint64_t a, page_flags b)
    {
        return a | static_cast<uint64_t>(b);
    }

    class vmm
    {
    public:
        void initialize();

        bool map(uint64_t virt, uint64_t phys, uint64_t flags);
        void unmap(uint64_t virt);

        uint64_t virtual_to_physical(uint64_t virt);
        uint64_t physical_to_virtual(uint64_t phys);

        void activate();

    private:
        uint64_t pml4_physical = 0;
        uint64_t hhdm_offset = 0;
        uint64_t* get_pte(uint64_t virt, bool create);
    };
} // namespace kt_kernel
