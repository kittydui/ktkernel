#pragma once

#include <cstdint>

struct kernel_symbol
{
    const char* name;
    void* address;
};

// clang-format off
#define kt_export_symbol(symbol) \
    static const kernel_symbol __ktsymbol_##symbol \
    __attribute__((used, section(".ktsymbols"), aligned(8))) = \
    { #symbol, reinterpret_cast<void*>(&symbol) }
// clang-format on
