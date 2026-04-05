#pragma once
#include <cstddef>
#include <cstdint>

namespace kt_kernel
{
    constexpr auto max_order = 25;
    constexpr auto min_order = 12;
    constexpr auto orders = max_order - min_order + 1;

    constexpr size_t bytes_to_kb(size_t x)
    {
        return (x) / (1024);
    }
    constexpr size_t bytes_to_mb(size_t x)
    {
        return (x) / (1024 * 1024);
    }
    constexpr size_t bytes_to_gb(size_t x)
    {
        return (x) / (1024 * 1024 * 1024);
    }
    constexpr size_t kb_to_bytes(size_t x)
    {
        return (x) * (1024);
    }
    constexpr size_t mb_to_bytes(size_t x)
    {
        return (x) * (1024 * 1024);
    }
    constexpr size_t gb_to_bytes(size_t x)
    {
        return (x) * (1024 * 1024 * 1024);
    }

    struct block
    {
        block* next;
    };

    inline size_t get_order(size_t size);
    inline size_t bit_length(size_t size);

    class pmm;

    class buddy_allocator
    {
        friend class pmm;

    private:
        block* free_lists[orders] = { nullptr };
        uint64_t hhdm_offset = 0;

        uint64_t get_buddy(uint64_t address, size_t order);
        void split(size_t order);
        void coalesce(uint64_t addr, size_t order);

    public:
        void add_region(uint64_t base, size_t size);
        uint64_t allocate_pages(size_t order);
        void free_pages(uint64_t addr, size_t order);
    };

    class pmm
    {
    public:
        bool initialize();

        uint64_t allocate_frame();
        uint64_t allocate_contiguous(size_t n);
        void free_frame(uint64_t frame);
        void free_contiguous(uint64_t frame, size_t n);

        size_t get_total_memory();
        size_t get_used_memory();

    private:
        buddy_allocator engine;
        uint64_t hhdm_offset = 0;
        size_t total_usable_pages = 0;
        size_t used_pages = 0;
    };
} // namespace kt_kernel
