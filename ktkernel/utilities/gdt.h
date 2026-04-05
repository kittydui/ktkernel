#pragma once
#include <cstdint>

namespace kt_kernel
{
    static uint64_t gdt[] = {
        0x0000000000000000,
        0x00009a000000ffff,
        0x000092000000ffff,
        0x00cf9a000000ffff,
        0x00cf92000000ffff,
        0x00a09a0000000000,
        0x0000920000000000,
        0x00a0fa0000000000,
        0x0000f20000000000,
    };

    constexpr uint16_t kernel_code_selector = 0x28;
    constexpr uint16_t kernel_data_selector = 0x30;

    bool setup_gdt();
} // namespace kt_kernel
