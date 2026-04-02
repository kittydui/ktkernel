#include "subsystems/console/console.h"
#include "limine/requests.h"
#include "mem/memory.h"
#include <cstdarg>

namespace KtCore
{
    bool Console::initialize(void* fontFile)
    {
        // by this time we know that the framebuffer request was valid.
        m_framebuffer = Limine::framebufferRequest.response->framebuffers[0];
        m_font = reinterpret_cast<PSF2Header*>(fontFile);
        return m_framebuffer != nullptr && m_font != nullptr;
    }

    void Console::shutdown()
    {
    }

    void Console::attachSerialPort(SerialPort* port)
    {
        m_serialPort = port;
    }

    void Console::drawCharacter(int x, int y, char c, uint32_t foreground, uint32_t background)
    {
        uint8_t* glyph = reinterpret_cast<uint8_t*>(m_font) + m_font->m_headerSize + (c * m_font->m_charSize);
        uint32_t bytes = (m_font->m_width + 7) / 8;

        for (uint32_t cy = 0; cy < m_font->m_height; cy++) {
            for (uint32_t cx = 0; cx < m_font->m_width; cx++) {
                bool bit = glyph[cx / 8] & (0x80 >> (cx % 8));
                auto* pixel =
                    reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(m_framebuffer->address) +
                                                (y + cy) * m_framebuffer->pitch + (x + cx) * (m_framebuffer->bpp / 8));
                *pixel = bit ? foreground : background;
            }
            glyph += bytes;
        }
    }

    void Console::clear(uint32_t color)
    {
        uint32_t* fb_ptr = reinterpret_cast<uint32_t*>(m_framebuffer->address);
        size_t pixels_per_line = m_framebuffer->pitch / (m_framebuffer->bpp / 8);
        size_t total_pixels = pixels_per_line * m_framebuffer->height;

        for (size_t i = 0; i < total_pixels; i++)
            fb_ptr[i] = color;

        this->m_cursorX = 0;
        this->m_cursorY = 0;
    }

    void Console::scroll()
    {
        size_t charHeight = m_font->m_height * m_framebuffer->pitch;
        size_t totalFramebuffer = m_framebuffer->height * m_framebuffer->pitch;

        memmove(m_framebuffer->address,
                reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_framebuffer->address) + charHeight),
                totalFramebuffer - charHeight);

        memset(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_framebuffer->address) + totalFramebuffer -
                                       charHeight),
               0,
               charHeight);

        m_cursorY -= m_font->m_height;
    }

    void Console::putChar(char c, uint32_t foreground, uint32_t background)
    {
        if (c == '\n') {
            m_cursorX = 0;
            m_cursorY += m_font->m_height;
        } else if (c == '\r') {
            m_cursorX = 0;
        } else if (c == '\t') {
            m_cursorX += (m_font->m_width * 4);
        } else {
            drawCharacter(m_cursorX, m_cursorY, c, foreground, background);
            m_cursorX += m_font->m_width;
        }

        if (m_cursorX + m_font->m_width > m_framebuffer->width) {
            m_cursorX = 0;
            m_cursorY += m_font->m_height;
        }

        if (m_cursorY + m_font->m_height > m_framebuffer->height)
            scroll();
    }

    void Console::print(const char* str, uint32_t foreground, uint32_t background)
    {
        while (*str) {
            char c = *str++;
            putChar(c, foreground, background);

            if (m_serialPort != nullptr)
                m_serialPort->writeChar(c);
        }
    }

    void Console::vprintf(const char* fmt, va_list args, uint32_t fg, uint32_t bg)
    {
        m_fmtBuffer.clear();
        Internal::FormatBuffer buf{ m_fmtBuffer };

        while (*fmt) {
            if (*fmt != '%') {
                buf.putChar(*fmt++);
                continue;
            }
            fmt++;

            if (*fmt == '%') {
                buf.putChar('%');
                fmt++;
                continue;
            }

            int longCount = 0;
            while (*fmt == 'l') {
                longCount++;
                fmt++;
            }

            switch (*fmt) {
            case 'd':
            case 'i': {
                int64_t val = (longCount >= 2) ? va_arg(args, long long)
                            : (longCount == 1) ? va_arg(args, long)
                                               : va_arg(args, int);
                if (val < 0) {
                    buf.putChar('-');
                    Internal::WriteDecimal(buf, -static_cast<uint64_t>(val));
                } else {
                    Internal::WriteDecimal(buf, static_cast<uint64_t>(val));
                }
                break;
            }
            case 'u': {
                uint64_t val = (longCount >= 2) ? va_arg(args, unsigned long long)
                             : (longCount == 1) ? va_arg(args, unsigned long)
                                                : va_arg(args, unsigned int);
                Internal::WriteDecimal(buf, val);
                break;
            }
            case 'x': {
                uint64_t val = (longCount >= 2) ? va_arg(args, unsigned long long)
                             : (longCount == 1) ? va_arg(args, unsigned long)
                                                : va_arg(args, unsigned int);
                Internal::WriteHex(buf, val);
                break;
            }
            case 's': {
                const char* s = va_arg(args, const char*);
                buf.putString(s ? s : "(null)");
                break;
            }
            case 'c': {
                char c = static_cast<char>(va_arg(args, int));
                buf.putChar(c);
                break;
            }
            case 'p': {
                void* p = va_arg(args, void*);
                buf.putString("0x");
                Internal::WriteHex(buf, reinterpret_cast<uintptr_t>(p));
                break;
            }
            default:
                buf.putChar('%');
                buf.putChar(*fmt);
                break;
            }
            fmt++;
        }

        buf.terminate();
        print(m_fmtBuffer.data(), fg, bg);
    }
} // namespace KtCore
