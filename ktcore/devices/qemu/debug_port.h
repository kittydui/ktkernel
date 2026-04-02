#pragma once
#include <kt/intrin.h>

namespace KtCore::Qemu
{
    constexpr auto ISA_DEBUG_EXIT_PORT = 0xF4;
    inline void Exit(uint8_t code)
    {
        outb(ISA_DEBUG_EXIT_PORT, code);
    }
} // namespace KtCore::Qemu
