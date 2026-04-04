#pragma once
#include <cstdint>

namespace KtKernel
{
    // Credit to lukflug of osdev.wiki Discord Server.
    static uint64_t GDT[] = {
        0x0000000000000000, // null
        0x00009a000000ffff, // 16-bit code
        0x000092000000ffff, // 16-bit data
        0x00cf9a000000ffff, // 32-bit code
        0x00cf92000000ffff, // 32-bit data
        0x00a09a0000000000, // 64-bit code
        0x0000920000000000, // 64-bit data
        0x00a0fa0000000000, // usermode 64-bit code
        0x0000f20000000000  // usermode 64-bit data
        /* at some stage (presumably when starting to work on usermode) you'll need at least one TSS desriptor
         * in order to set that up, you'll need some knowledge of the segment descriptor and TSS layout */
    };

    constexpr uint16_t KERNEL_CODE_SELECTOR = 0x28;
    constexpr uint16_t KERNEL_DATA_SELECTOR = 0x30;

    bool SetupGdt();
} // namespace KtKernel
