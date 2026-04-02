#pragma once
#include "limine/limine.h"
#include "mem/vector.h"
#include "subsystems/console/format.h"
#include "subsystems/console/serial.h"
#include <cstdarg>

namespace KtCore
{
    enum ConsoleColor : uint32_t
    {
        // ARGB
        CC_BLACK = 0xFF000000,
        CC_WHITE = 0xFFFFFFFF,
        CC_RED = 0xFFFF0000,
        CC_GREEN = 0xFF00FF00,
        CC_BLUE = 0xFF0000FF
    };

    struct PSF2Header
    {
        uint32_t m_magic;
        uint32_t m_version;
        uint32_t m_headerSize;
        uint32_t m_flags;
        uint32_t m_length;
        uint32_t m_charSize;
        uint32_t m_height;
        uint32_t m_width;
    };

    class Console
    {
    public:
        bool initialize(void* fontFile);
        void shutdown();

        void attachSerialPort(SerialPort* port);

        void putChar(char c, uint32_t foreground, uint32_t background);
        void print(const char* str, uint32_t foreground = CC_WHITE, uint32_t background = CC_BLACK);
        void clear(uint32_t color = CC_BLACK);
        void vprintf(const char* fmt, va_list args, uint32_t fg = CC_WHITE, uint32_t bg = CC_BLACK);

        template <typename... Args> void printf(const char* format, Args... args)
        {
            printf(CC_WHITE, CC_BLACK, format, args...);
        }

        template <typename... Args> void printf(uint32_t fg, uint32_t bg, const char* format, Args... args)
        {
            m_fmtBuffer.clear();
            Internal::FormatBuffer buf{ m_fmtBuffer };
            Internal::FormatImpl(buf, format, args...);
            buf.terminate();
            print(m_fmtBuffer.data(), fg, bg);
        }

    private:
        void scroll();
        void drawCharacter(int x, int y, char c, uint32_t foreground, uint32_t background);

        SerialPort* m_serialPort = nullptr;

        limine_framebuffer* m_framebuffer = nullptr;
        PSF2Header* m_font = nullptr;
        uint32_t m_cursorX = 0;
        uint32_t m_cursorY = 0;
        Vector<char> m_fmtBuffer;
    };

} // namespace KtCore
