#pragma once
#include <cstddef>
#include <cstdint>

namespace KtCore {
namespace Internal {

struct FormatBuffer {
    char* data;
    size_t capacity;
    size_t pos;

    void putChar(char c)
    {
        if (pos < capacity - 1)
            data[pos++] = c;
    }

    void putString(const char* s)
    {
        while (*s && pos < capacity - 1)
            data[pos++] = *s++;
    }

    void terminate() { data[pos] = '\0'; }
};

inline void WriteDecimal(FormatBuffer& buf, uint64_t value)
{
    if (value == 0) {
        buf.putChar('0');
        return;
    }
    char tmp[20];
    int len = 0;
    while (value > 0) {
        tmp[len++] = '0' + (value % 10);
        value /= 10;
    }
    for (int i = len - 1; i >= 0; i--)
        buf.putChar(tmp[i]);
}

inline void WriteHex(FormatBuffer& buf, uint64_t value)
{
    if (value == 0) {
        buf.putChar('0');
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
        buf.putChar(tmp[i]);
}

inline void FormatArg(FormatBuffer& buf, const char* v) { buf.putString(v); }
inline void FormatArg(FormatBuffer& buf, char v) { buf.putChar(v); }
inline void FormatArg(FormatBuffer& buf, bool v) { buf.putString(v ? "true" : "false"); }

inline void FormatArg(FormatBuffer& buf, int v)
{
    if (v < 0) {
        buf.putChar('-');
        v = -v;
    }
    WriteDecimal(buf, static_cast<uint64_t>(v));
}

inline void FormatArg(FormatBuffer& buf, unsigned int v) { WriteDecimal(buf, v); }

inline void FormatArg(FormatBuffer& buf, long v)
{
    if (v < 0) {
        buf.putChar('-');
        WriteDecimal(buf, -static_cast<uint64_t>(v));
    } else {
        WriteDecimal(buf, static_cast<uint64_t>(v));
    }
}

inline void FormatArg(FormatBuffer& buf, unsigned long v) { WriteDecimal(buf, v); }

inline void FormatArg(FormatBuffer& buf, long long v)
{
    if (v < 0) {
        buf.putChar('-');
        WriteDecimal(buf, -static_cast<uint64_t>(v));
    } else {
        WriteDecimal(buf, static_cast<uint64_t>(v));
    }
}

inline void FormatArg(FormatBuffer& buf, unsigned long long v) { WriteDecimal(buf, v); }

inline void FormatArg(FormatBuffer& buf, const void* v)
{
    buf.putString("0x");
    WriteHex(buf, reinterpret_cast<uintptr_t>(v));
}

inline void FormatArgHex(FormatBuffer& buf, const char* v) { buf.putString(v); }
inline void FormatArgHex(FormatBuffer& buf, char v) { WriteHex(buf, static_cast<uint8_t>(v)); }
inline void FormatArgHex(FormatBuffer& buf, bool v) { FormatArg(buf, v); }
inline void FormatArgHex(FormatBuffer& buf, int v) { WriteHex(buf, static_cast<uint32_t>(v)); }
inline void FormatArgHex(FormatBuffer& buf, unsigned int v) { WriteHex(buf, v); }
inline void FormatArgHex(FormatBuffer& buf, long v) { WriteHex(buf, static_cast<uint64_t>(v)); }
inline void FormatArgHex(FormatBuffer& buf, unsigned long v) { WriteHex(buf, v); }
inline void FormatArgHex(FormatBuffer& buf, long long v) { WriteHex(buf, static_cast<uint64_t>(v)); }
inline void FormatArgHex(FormatBuffer& buf, unsigned long long v) { WriteHex(buf, v); }
inline void FormatArgHex(FormatBuffer& buf, const void* v) { WriteHex(buf, reinterpret_cast<uintptr_t>(v)); }

inline void FormatImpl(FormatBuffer& buf, const char* fmt) { buf.putString(fmt); }

template <typename T, typename... Rest> void FormatImpl(FormatBuffer& buf, const char* fmt, T value, Rest... rest)
{
    while (*fmt) {
        if (fmt[0] == '{') {
            if (fmt[1] == '}') {
                FormatArg(buf, value);
                FormatImpl(buf, fmt + 2, rest...);
                return;
            }
            if (fmt[1] == 'x' && fmt[2] == '}') {
                FormatArgHex(buf, value);
                FormatImpl(buf, fmt + 3, rest...);
                return;
            }
        }
        buf.putChar(*fmt);
        fmt++;
    }
}

} // namespace Internal
} // namespace KtCore
