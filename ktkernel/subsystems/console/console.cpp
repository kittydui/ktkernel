#include "subsystems/console/console.h"
#include "limine/requests.h"
#include "mem/memory.h"
#include <cstdarg>

namespace kt_kernel
{
    bool console::initialize(void* font_file)
    {
        framebuffer = limine::framebuffer_request.response->framebuffers[0];
        font = reinterpret_cast<psf2_header*>(font_file);
        return framebuffer != nullptr && font != nullptr;
    }

    void console::shutdown()
    {
    }

    void console::attach_serial_port(serial_port* port)
    {
        serial = port;
    }

    void console::draw_character(int x, int y, char c, uint32_t foreground, uint32_t background)
    {
        uint8_t* glyph = reinterpret_cast<uint8_t*>(font) + font->header_size + (c * font->char_size);
        uint32_t bytes = (font->width + 7) / 8;

        for (uint32_t cy = 0; cy < font->height; cy++) {
            for (uint32_t cx = 0; cx < font->width; cx++) {
                bool bit = glyph[cx / 8] & (0x80 >> (cx % 8));
                auto* pixel =
                    reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(framebuffer->address) +
                                                (y + cy) * framebuffer->pitch + (x + cx) * (framebuffer->bpp / 8));
                *pixel = bit ? foreground : background;
            }
            glyph += bytes;
        }
    }

    void console::clear(uint32_t color)
    {
        uint32_t* fb_ptr = reinterpret_cast<uint32_t*>(framebuffer->address);
        size_t pixels_per_line = framebuffer->pitch / (framebuffer->bpp / 8);
        size_t total_pixels = pixels_per_line * framebuffer->height;

        for (size_t i = 0; i < total_pixels; i++)
            fb_ptr[i] = color;

        this->cursor_x = 0;
        this->cursor_y = 0;
    }

    void console::scroll()
    {
        size_t char_height = font->height * framebuffer->pitch;
        size_t total_framebuffer = framebuffer->height * framebuffer->pitch;

        memmove(framebuffer->address,
                reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(framebuffer->address) + char_height),
                total_framebuffer - char_height);

        memset(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(framebuffer->address) + total_framebuffer -
                                       char_height),
               0,
               char_height);

        cursor_y -= font->height;
    }

    void console::put_char(char c, uint32_t foreground, uint32_t background)
    {
        if (c == '\n') {
            cursor_x = 0;
            cursor_y += font->height;
        } else if (c == '\r') {
            cursor_x = 0;
        } else if (c == '\t') {
            cursor_x += (font->width * 4);
        } else {
            draw_character(cursor_x, cursor_y, c, foreground, background);
            cursor_x += font->width;
        }

        if (cursor_x + font->width > framebuffer->width) {
            cursor_x = 0;
            cursor_y += font->height;
        }

        if (cursor_y + font->height > framebuffer->height)
            scroll();
    }

    void console::print(const char* str, uint32_t foreground, uint32_t background)
    {
        while (*str) {
            char c = *str++;
            put_char(c, foreground, background);

            if (serial != nullptr)
                serial->write_char(c);
        }
    }

    void console::v_printf(const char* fmt, va_list args, uint32_t fg, uint32_t bg)
    {
        fmt_buffer.clear();
        internal::format_buffer buf{ fmt_buffer };

        while (*fmt) {
            if (*fmt != '%') {
                buf.put_char(*fmt++);
                continue;
            }
            fmt++;

            if (*fmt == '%') {
                buf.put_char('%');
                fmt++;
                continue;
            }

            int long_count = 0;
            while (*fmt == 'l') {
                long_count++;
                fmt++;
            }

            switch (*fmt) {
            case 'd':
            case 'i': {
                int64_t val = (long_count >= 2) ? va_arg(args, long long)
                            : (long_count == 1) ? va_arg(args, long)
                                                : va_arg(args, int);
                if (val < 0) {
                    buf.put_char('-');
                    internal::write_decimal(buf, -static_cast<uint64_t>(val));
                } else {
                    internal::write_decimal(buf, static_cast<uint64_t>(val));
                }
                break;
            }
            case 'u': {
                uint64_t val = (long_count >= 2) ? va_arg(args, unsigned long long)
                             : (long_count == 1) ? va_arg(args, unsigned long)
                                                : va_arg(args, unsigned int);
                internal::write_decimal(buf, val);
                break;
            }
            case 'x': {
                uint64_t val = (long_count >= 2) ? va_arg(args, unsigned long long)
                             : (long_count == 1) ? va_arg(args, unsigned long)
                                                : va_arg(args, unsigned int);
                internal::write_hex(buf, val);
                break;
            }
            case 's': {
                const char* s = va_arg(args, const char*);
                buf.put_string(s ? s : "(null)");
                break;
            }
            case 'c': {
                char c = static_cast<char>(va_arg(args, int));
                buf.put_char(c);
                break;
            }
            case 'p': {
                void* p = va_arg(args, void*);
                buf.put_string("0x");
                internal::write_hex(buf, reinterpret_cast<uintptr_t>(p));
                break;
            }
            default:
                buf.put_char('%');
                buf.put_char(*fmt);
                break;
            }
            fmt++;
        }

        buf.terminate();
        print(fmt_buffer.data(), fg, bg);
    }
} // namespace kt_kernel
