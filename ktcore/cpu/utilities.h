#pragma once
#include <cstdint>

namespace KtCore
{
[[noreturn]] inline void Halt()
{
    while (true)
        asm volatile("hlt");
}

inline void Invlpg(uint64_t addr)
{
    asm volatile("invlpg (%0)" ::"r"(addr) : "memory");
}

inline uint64_t ReadCR3()
{
    uint64_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

inline void WriteCR3(uint64_t cr3)
{
    asm volatile("mov %0, %%cr3" ::"r"(cr3) : "memory");
}

} // namespace KtCore
