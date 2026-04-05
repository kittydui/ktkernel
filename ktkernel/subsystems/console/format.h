#pragma once
#include "mem/vector.h"
#include <cstddef>
#include <cstdint>

namespace kt_kernel::internal
{
    struct format_buffer
    {
        vector<char>& out;

        explicit format_buffer(vector<char>& buf) : out(buf)
        {
        }

        void put_char(char c)
        {
            out.push_back(c);
        }

        void put_string(const char* s)
        {
            while (*s)
                out.push_back(*s++);
        }

        void terminate()
        {
            out.push_back('\0');
        }
    };

    inline void write_decimal(format_buffer& buf, uint64_t value)
    {
        if (value == 0) {
            buf.put_char('0');
            return;
        }
        char tmp[20];
        int len = 0;
        while (value > 0) {
            tmp[len++] = '0' + (value % 10);
            value /= 10;
        }
        for (int i = len - 1; i >= 0; i--)
            buf.put_char(tmp[i]);
    }

    inline void write_hex(format_buffer& buf, uint64_t value)
    {
        if (value == 0) {
            buf.put_char('0');
            return;
        }
        constexpr char digits[] = "0123456789abcdef";
        char tmp[16];
        int len = 0;
        while (value > 0) {
            tmp[len++] = digits[value & 0xF];
            value >>= 4;
        }
        for (int i = len - 1; i >= 0; i--)
            buf.put_char(tmp[i]);
    }

    inline void format_arg(format_buffer& buf, const char* v)
    {
        buf.put_string(v);
    }
    inline void format_arg(format_buffer& buf, char v)
    {
        buf.put_char(v);
    }
    inline void format_arg(format_buffer& buf, bool v)
    {
        buf.put_string(v ? "true" : "false");
    }

    inline void format_arg(format_buffer& buf, int v)
    {
        if (v < 0) {
            buf.put_char('-');
            v = -v;
        }
        write_decimal(buf, static_cast<uint64_t>(v));
    }

    inline void format_arg(format_buffer& buf, unsigned int v)
    {
        write_decimal(buf, v);
    }

    inline void format_arg(format_buffer& buf, long v)
    {
        if (v < 0) {
            buf.put_char('-');
            write_decimal(buf, -static_cast<uint64_t>(v));
        } else {
            write_decimal(buf, static_cast<uint64_t>(v));
        }
    }

    inline void format_arg(format_buffer& buf, unsigned long v)
    {
        write_decimal(buf, v);
    }

    inline void format_arg(format_buffer& buf, long long v)
    {
        if (v < 0) {
            buf.put_char('-');
            write_decimal(buf, -static_cast<uint64_t>(v));
        } else {
            write_decimal(buf, static_cast<uint64_t>(v));
        }
    }

    inline void format_arg(format_buffer& buf, unsigned long long v)
    {
        write_decimal(buf, v);
    }

    inline void format_arg(format_buffer& buf, const void* v)
    {
        buf.put_string("0x");
        write_hex(buf, reinterpret_cast<uintptr_t>(v));
    }

    inline void format_arg_hex(format_buffer& buf, const char* v)
    {
        buf.put_string(v);
    }
    inline void format_arg_hex(format_buffer& buf, char v)
    {
        write_hex(buf, static_cast<uint8_t>(v));
    }
    inline void format_arg_hex(format_buffer& buf, bool v)
    {
        format_arg(buf, v);
    }
    inline void format_arg_hex(format_buffer& buf, int v)
    {
        write_hex(buf, static_cast<uint32_t>(v));
    }
    inline void format_arg_hex(format_buffer& buf, unsigned int v)
    {
        write_hex(buf, v);
    }
    inline void format_arg_hex(format_buffer& buf, long v)
    {
        write_hex(buf, static_cast<uint64_t>(v));
    }
    inline void format_arg_hex(format_buffer& buf, unsigned long v)
    {
        write_hex(buf, v);
    }
    inline void format_arg_hex(format_buffer& buf, long long v)
    {
        write_hex(buf, static_cast<uint64_t>(v));
    }
    inline void format_arg_hex(format_buffer& buf, unsigned long long v)
    {
        write_hex(buf, v);
    }
    inline void format_arg_hex(format_buffer& buf, const void* v)
    {
        write_hex(buf, reinterpret_cast<uintptr_t>(v));
    }

    inline void format_impl(format_buffer& buf, const char* fmt)
    {
        buf.put_string(fmt);
    }

    template <typename T, typename... Rest> void format_impl(format_buffer& buf, const char* fmt, T value, Rest... rest)
    {
        while (*fmt) {
            if (fmt[0] == '{') {
                if (fmt[1] == '}') {
                    format_arg(buf, value);
                    format_impl(buf, fmt + 2, rest...);
                    return;
                }
                if (fmt[1] == 'x' && fmt[2] == '}') {
                    format_arg_hex(buf, value);
                    format_impl(buf, fmt + 3, rest...);
                    return;
                }
            }
            buf.put_char(*fmt);
            fmt++;
        }
    }
} // namespace kt_kernel::internal
