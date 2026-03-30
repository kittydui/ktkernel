#pragma once
#include <cstdint>

// Breaking standard here but PascalCase for inline assembly functions
// look horrible, so im just gonna do camelCase with no namespace.

[[noreturn]] inline void halt()
{
    while (true)
        asm volatile("hlt");
}

inline void invlpg(uint64_t addr)
{
    asm volatile("invlpg (%0)" ::"r"(addr) : "memory");
}

inline uint64_t readCr3()
{
    uint64_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

inline void writeCr3(uint64_t cr3)
{
    asm volatile("mov %0, %%cr3" ::"r"(cr3) : "memory");
}

inline void outb(uint16_t port, uint8_t val)
{
    asm volatile("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

inline bool areInterruptsEnabled()
{
    unsigned long flags;
    // 're' is a more specific constraint for flags, but 'g' works
    asm volatile("pushf; pop %0" : "=rm"(flags) : : "memory");
    return (flags & (1 << 9)) != 0;
}

inline void ioWait()
{
    outb(0x80, 0);
}
