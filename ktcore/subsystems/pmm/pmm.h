#pragma once
#include <cstddef>
#include <cstdint>

namespace KtCore
{
constexpr auto MAX_ORDER = 25;
constexpr auto MIN_ORDER = 12;
constexpr auto ORDERS = MAX_ORDER - MIN_ORDER + 1;

constexpr size_t BytesToKB(size_t x)
{
    return (x) / (1024);
}
constexpr size_t BytesToMB(size_t x)
{
    return (x) / (1024 * 1024);
}
constexpr size_t BytesToGB(size_t x)
{
    return (x) / (1024 * 1024 * 1024);
}
constexpr size_t KBToBytes(size_t x)
{
    return (x) * (1024);
}
constexpr size_t MBToBytes(size_t x)
{
    return (x) * (1024 * 1024);
}
constexpr size_t GBToBytes(size_t x)
{
    return (x) * (1024 * 1024 * 1024);
}

struct Block
{
    Block* m_next;
};

inline size_t GetOrder(size_t size);
inline size_t BitLength(size_t size);

class PMM;

class BuddyAllocator
{
private:
    Block* m_freeLists[ORDERS] = { nullptr };
    uint64_t m_hhdmOffset;

    uint64_t getBuddy(uint64_t address, size_t order);
    void split(size_t order);
    void coalesce(uint64_t addr, size_t order);

    friend class PMM;

public:
    void addRegion(uint64_t base, size_t size);
    uint64_t allocatePages(size_t order);
    void freePages(uint64_t addr, size_t order);
};

class PMM
{
public:
    bool initialize();

    uint64_t allocateFrame();
    uint64_t allocateContiguous(size_t n);
    void freeFrame(uint64_t frame);
    void freeContiguous(uint64_t frame, size_t n);

    size_t getTotalMemory();
    size_t getUsedMemory();

private:
    BuddyAllocator m_allocatorEngine;
    uint64_t m_hhdmOffset = 0;
    size_t m_totalUsablePages = 0;
    size_t m_usedPages = 0;
};
} // namespace KtCore
