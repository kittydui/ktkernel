#pragma once
#include <cstdint>

namespace kt_kernel
{
    enum class interrupt_vector : uint8_t
    {
        bp = 3,
        gp = 13,
    };

    void int3_handler(uint8_t interrupt_index, uint64_t error_code);
    void gp_handler(uint8_t interrupt_index, uint64_t error_code);

    [[noreturn]] void exception_handler();

} // namespace kt_kernel
