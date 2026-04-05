#pragma once
#include "limine/limine.h"
#include "mem/vector.h"
#include "subsystems/console/format.h"
#include "subsystems/console/serial.h"
#include <cstdarg>

namespace kt_kernel
{
    enum class console_color : uint32_t
    {
        black = 0xFF000000,
        white = 0xFFFFFFFF,
        red = 0xFFFF0000,
        green = 0xFF00FF00,
        blue = 0xFF0000FF
    };

    struct psf2_header
    {
        uint32_t magic;
        uint32_t version;
        uint32_t header_size;
        uint32_t flags;
        uint32_t length;
        uint32_t char_size;
        uint32_t height;
        uint32_t width;
    };

    class console
    {
    public:
        bool initialize(void* font_file);
        void shutdown();

        void attach_serial_port(serial_port* port);

        void put_char(char c, uint32_t foreground, uint32_t background);
        void print(const char* str, uint32_t foreground = static_cast<uint32_t>(console_color::white),
                    uint32_t background = static_cast<uint32_t>(console_color::black));
        void clear(uint32_t color = static_cast<uint32_t>(console_color::black));
        void v_printf(const char* fmt, va_list args, uint32_t fg = static_cast<uint32_t>(console_color::white),
                      uint32_t bg = static_cast<uint32_t>(console_color::black));

        template <typename... Args> void printf(const char* format, Args... args)
        {
            printf(static_cast<uint32_t>(console_color::white), static_cast<uint32_t>(console_color::black), format,
                   args...);
        }

        template <typename... Args> void printf(uint32_t fg, uint32_t bg, const char* format, Args... args)
        {
            fmt_buffer.clear();
            internal::format_buffer buf{ fmt_buffer };
            internal::format_impl(buf, format, args...);
            buf.terminate();
            print(fmt_buffer.data(), fg, bg);
        }

    private:
        void scroll();
        void draw_character(int x, int y, char c, uint32_t foreground, uint32_t background);

        serial_port* serial = nullptr;

        limine_framebuffer* framebuffer = nullptr;
        psf2_header* font = nullptr;
        uint32_t cursor_x = 0;
        uint32_t cursor_y = 0;
        vector<char> fmt_buffer;
    };

} // namespace kt_kernel
