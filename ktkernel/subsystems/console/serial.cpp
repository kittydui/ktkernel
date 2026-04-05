#include "subsystems/console/serial.h"
#include <kt/intrin.h>

namespace kt_kernel
{
    bool serial_port::initialize(uint16_t port)
    {
        io_port = port;

        outb(port + 1, 0x00);
        outb(port + 3, 0x80);
        outb(port + 0, 0x03);
        outb(port + 1, 0x00);
        outb(port + 3, 0x03);
        outb(port + 2, 0xC7);
        outb(port + 4, 0x0B);
        outb(port + 4, 0x1E);
        outb(port + 0, 0xAE);

        if (inb(port) != 0xAE)
            return false;

        outb(port + 4, 0x0F);
        return true;
    }

    void serial_port::write_char(char c)
    {
        while ((inb(io_port + 5) & 0x20) == 0)
            ;

        outb(io_port, c);
    }

    char serial_port::read_char()
    {
        while ((inb(io_port + 5) & 1) == 0)
            ;

        return inb(io_port);
    }
} // namespace kt_kernel
