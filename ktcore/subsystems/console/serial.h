#pragma once
#include <cstdint>

namespace KtCore
{
    constexpr auto COM1_PORT = 0x3F8;

    class SerialPort
    {
    public:
        bool initialize(uint16_t serialPort);

        void writeChar(char c);
        char readChar();

        uint16_t m_serialPort;
    };
} // namespace KtCore
