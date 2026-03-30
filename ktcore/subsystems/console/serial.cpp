#include "subsystems/console/serial.h"
#include "cpu/utilities.h"

namespace KtCore
{
    bool SerialPort::initialize(uint16_t serialPort)
    {
        // https://osdev.wiki/wiki/Serial_Ports#Example_Code
        m_serialPort = serialPort;

        outb(serialPort + 1, 0x00);
        outb(serialPort + 3, 0x80); // Enable DLAB (set baud rate divisor)
        outb(serialPort + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
        outb(serialPort + 1, 0x00); //                  (hi byte)
        outb(serialPort + 3, 0x03); // 8 bits, no parity, one stop bit
        outb(serialPort + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
        outb(serialPort + 4, 0x0B); // IRQs enabled, RTS/DSR set
        outb(serialPort + 4, 0x1E); // Set in loopback mode, test the serial chip
        outb(serialPort + 0, 0xAE);

        if (inb(serialPort) != 0xAE)
            return false;

        outb(serialPort + 4, 0x0F);
        return true;
    }

    void SerialPort::writeChar(char c)
    {
        while ((inb(m_serialPort + 5) & 0x20) == 0)
            ;

        outb(m_serialPort, c);
    }

    char SerialPort::readChar()
    {
        while ((inb(m_serialPort + 5) & 1) == 0)
            ;

        return inb(m_serialPort);
    }
} // namespace KtCore
