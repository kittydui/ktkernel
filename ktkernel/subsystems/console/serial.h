#pragma once
#include <cstdint>

namespace kt_kernel
{
    constexpr auto com1_port = 0x3F8;

    class serial_port
    {
    public:
        bool initialize(uint16_t port);

        void write_char(char c);
        char read_char();

        uint16_t io_port = 0;
    };
} // namespace kt_kernel
