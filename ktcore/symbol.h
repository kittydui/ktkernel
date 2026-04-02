#pragma once

#include <cstdint>

struct KernelSymbol
{
    const char* m_name;
    void* m_address;
};

// clang-format off
#define KtExportSymbol(symbol) \
    static const KernelSymbol __ktsymbol_##symbol \
    __attribute__((used, section(".ktsymbols"), aligned(8))) = \
    { #symbol, reinterpret_cast<void*>(&symbol) }
// clang-format on
