#include "subsystems/pmm/pmm.h"
#include "limine/requests.h"

namespace kt_kernel
{
    constexpr auto min_block_size = 1ull << min_order;

    inline size_t get_order(size_t size)
    {
        if (size <= min_block_size)
            return 0;

        return bit_length(size - 1) - bit_length(min_block_size - 1);
    }

    inline size_t bit_length(size_t size)
    {
        if (size == 0) [[unlikely]]
            return 0;

        return (sizeof(size) * 8) - __builtin_clzll(size);
    }

    uint64_t buddy_allocator::get_buddy(uint64_t address, size_t order)
    {
        return address ^ (1ull << (order + min_order));
    }

    void buddy_allocator::add_region(uint64_t base, size_t size)
    {
        base = (base + min_block_size - 1) & ~(min_block_size - 1);
        size &= ~(min_block_size - 1);

        while (size >= min_block_size) {
            size_t order = orders - 1;
            while (order > 0 && (1ull << (order + min_order)) > size)
                order--;
            while (order > 0 && (base & ((1ull << (order + min_order)) - 1)) != 0)
                order--;

            size_t block_size = 1ull << (order + min_order);
            auto* blk = reinterpret_cast<block*>(base + hhdm_offset);
            blk->next = free_lists[order];
            free_lists[order] = blk;

            base += block_size;
            size -= block_size;
        }
    }

    void buddy_allocator::split(size_t order)
    {
        block* blk = free_lists[order];
        free_lists[order] = blk->next;

        size_t half_size = 1ull << ((order - 1) + min_order);
        auto* second = reinterpret_cast<block*>(reinterpret_cast<uint64_t>(blk) + half_size);
        second->next = free_lists[order - 1];
        blk->next = second;
        free_lists[order - 1] = blk;
    }

    uint64_t buddy_allocator::allocate_pages(size_t order)
    {
        size_t current = order;
        while (current < orders && free_lists[current] == nullptr)
            current++;

        if (current >= orders)
            return 0;

        while (current > order) {
            split(current);
            current--;
        }

        block* blk = free_lists[order];
        free_lists[order] = blk->next;

        return reinterpret_cast<uint64_t>(blk) - hhdm_offset;
    }

    void buddy_allocator::coalesce(uint64_t addr, size_t order)
    {
        while (order < orders - 1) {
            uint64_t buddy_physical = get_buddy(addr, order);
            auto* buddy_virtual = reinterpret_cast<block*>(buddy_physical + hhdm_offset);

            block** prev = &free_lists[order];
            block* curr = free_lists[order];

            bool found = false;

            while (curr) {
                if (curr == buddy_virtual) {
                    *prev = curr->next;
                    found = true;
                    break;
                }

                prev = &curr->next;
                curr = curr->next;
            }

            if (!found)
                break;

            addr = (addr < buddy_physical) ? addr : buddy_physical;
            order++;
        }

        auto* blk = reinterpret_cast<block*>(addr + hhdm_offset);
        blk->next = free_lists[order];
        free_lists[order] = blk;
    }

    void buddy_allocator::free_pages(uint64_t addr, size_t order)
    {
        coalesce(addr, order);
    }

    bool pmm::initialize()
    {
        this->hhdm_offset = limine::hhdm_request.response->offset;
        engine.hhdm_offset = this->hhdm_offset;

        for (uint64_t i = 0; i < limine::memmap_request.response->entry_count; i++) {
            auto* entry = limine::memmap_request.response->entries[i];
            if (entry->type == LIMINE_MEMMAP_USABLE) {
                engine.add_region(entry->base, entry->length);
                total_usable_pages += (entry->length / 4096);
            }
        }

        return true;
    }

    uint64_t pmm::allocate_frame()
    {
        uint64_t frame = engine.allocate_pages(0);
        if (frame)
            used_pages++;

        return frame;
    }

    uint64_t pmm::allocate_contiguous(size_t n)
    {
        if (n == 0)
            return 0;

        size_t order = get_order(n * 4096);
        uint64_t frame = engine.allocate_pages(order);

        if (frame)
            used_pages += (1ull << order);

        return frame;
    }

    void pmm::free_frame(uint64_t frame)
    {
        engine.free_pages(frame, 0);
        if (used_pages > 0)
            used_pages--;
    }

    void pmm::free_contiguous(uint64_t frame, size_t n)
    {
        size_t order = get_order(n * min_block_size);
        engine.free_pages(frame, order);
        if (used_pages >= (1ull << order))
            used_pages -= (1ull << order);
    }

    size_t pmm::get_total_memory()
    {
        return total_usable_pages * 4096;
    }

    size_t pmm::get_used_memory()
    {
        return used_pages * 4096;
    }
} // namespace kt_kernel
