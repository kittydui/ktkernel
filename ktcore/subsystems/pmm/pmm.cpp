#include "subsystems/pmm/pmm.h"
#include "limine/requests.h"

namespace KtCore
{
    constexpr auto MIN_BLOCK_SIZE = 1ull << MIN_ORDER;

    inline size_t GetOrder(size_t size)
    {
        if (size <= MIN_BLOCK_SIZE)
            return 0;

        return BitLength(size - 1) - BitLength(MIN_BLOCK_SIZE - 1);
    }

    inline size_t BitLength(size_t size)
    {
        if (size == 0) [[unlikely]]
            return 0;

        return (sizeof(size) * 8) - __builtin_clzll(size);
    }

    // Buddy Allocator

    uint64_t BuddyAllocator::getBuddy(uint64_t address, size_t order)
    {
        return address ^ (1ull << (order + MIN_ORDER));
    }

    void BuddyAllocator::addRegion(uint64_t base, size_t size)
    {
        base = (base + MIN_BLOCK_SIZE - 1) & ~(MIN_BLOCK_SIZE - 1);
        size &= ~(MIN_BLOCK_SIZE - 1);

        while (size >= MIN_BLOCK_SIZE) {
            size_t order = ORDERS - 1;
            while (order > 0 && (1ull << (order + MIN_ORDER)) > size)
                order--;
            while (order > 0 && (base & ((1ull << (order + MIN_ORDER)) - 1)) != 0)
                order--;

            size_t block_size = 1ull << (order + MIN_ORDER);
            auto* block = reinterpret_cast<Block*>(base + m_hhdmOffset);
            block->m_next = m_freeLists[order];
            m_freeLists[order] = block;

            base += block_size;
            size -= block_size;
        }
    }

    void BuddyAllocator::split(size_t order)
    {
        Block* block = m_freeLists[order];
        m_freeLists[order] = block->m_next;

        size_t half_size = 1ull << ((order - 1) + MIN_ORDER);
        auto* second = reinterpret_cast<Block*>(reinterpret_cast<uint64_t>(block) + half_size);
        second->m_next = m_freeLists[order - 1];
        block->m_next = second;
        m_freeLists[order - 1] = block;
    }

    uint64_t BuddyAllocator::allocatePages(size_t order)
    {
        size_t current = order;
        while (current < ORDERS && m_freeLists[current] == nullptr)
            current++;

        if (current >= ORDERS)
            return 0;

        while (current > order) {
            split(current);
            current--;
        }

        Block* block = m_freeLists[order];
        m_freeLists[order] = block->m_next;

        return reinterpret_cast<uint64_t>(block) - m_hhdmOffset;
    }

    void BuddyAllocator::coalesce(uint64_t addr, size_t order)
    {
        while (order < ORDERS - 1) {
            uint64_t buddy_physical = getBuddy(addr, order);
            auto* buddy_virtual = reinterpret_cast<Block*>(buddy_physical + m_hhdmOffset);

            Block** prev = &m_freeLists[order];
            Block* curr = m_freeLists[order];

            bool found = false;

            while (curr) {
                if (curr == buddy_virtual) {
                    *prev = curr->m_next;
                    found = true;
                    break;
                }

                prev = &curr->m_next;
                curr = curr->m_next;
            }

            if (!found)
                break;

            addr = (addr < buddy_physical) ? addr : buddy_physical;
            order++;
        }

        auto* block = reinterpret_cast<Block*>(addr + m_hhdmOffset);
        block->m_next = m_freeLists[order];
        m_freeLists[order] = block;
    }

    void BuddyAllocator::freePages(uint64_t addr, size_t order)
    {
        coalesce(addr, order);
    }

    // PMM

    bool PMM::initialize()
    {
        this->m_hhdmOffset = Limine::hhdmRequest.response->offset;
        m_allocatorEngine.m_hhdmOffset = this->m_hhdmOffset;

        for (uint64_t i = 0; i < Limine::memmapRequest.response->entry_count; i++) {
            auto* entry = Limine::memmapRequest.response->entries[i];
            if (entry->type == LIMINE_MEMMAP_USABLE) {
                m_allocatorEngine.addRegion(entry->base, entry->length);
                m_totalUsablePages += (entry->length / 4096);
            }
        }

        return true;
    }

    uint64_t PMM::allocateFrame()
    {
        uint64_t frame = m_allocatorEngine.allocatePages(0);
        if (frame)
            m_usedPages++;

        return frame;
    }

    uint64_t PMM::allocateContiguous(size_t n)
    {
        if (n == 0)
            return 0;

        size_t order = GetOrder(n * 4096);
        uint64_t frame = m_allocatorEngine.allocatePages(order);

        if (frame)
            m_usedPages += (1ull << order);

        return frame;
    }

    void PMM::freeFrame(uint64_t frame)
    {
        m_allocatorEngine.freePages(frame, 0);
        if (m_usedPages > 0)
            m_usedPages--;
    }

    void PMM::freeContiguous(uint64_t frame, size_t n)
    {
        size_t order = GetOrder(n * MIN_BLOCK_SIZE);
        m_allocatorEngine.freePages(frame, order);
        if (m_usedPages >= (1ull << order))
            m_usedPages -= (1ull << order);
    }

    size_t PMM::getTotalMemory()
    {
        return m_totalUsablePages * 4096;
    }

    size_t PMM::getUsedMemory()
    {
        return m_usedPages * 4096;
    }
} // namespace KtCore
