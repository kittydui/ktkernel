#include "mem/memory.h"
#include <cstdint>

extern "C"
{
void* memcpy(void* dest, const void* src, size_t n)
{
    auto* d = reinterpret_cast<uint8_t*>(dest);
    const auto* s = reinterpret_cast<const uint8_t*>(src);
    while (n--)
        *d++ = *s++;

    return dest;
}

void* memset(void* s, int c, size_t n)
{
    auto* p = reinterpret_cast<uint8_t*>(s);
    while (n--)
        *p++ = (uint8_t)c;

    return s;
}

void* memmove(void* dest, const void* src, size_t n)
{
    auto* d = reinterpret_cast<uint8_t*>(dest);
    const auto* s = reinterpret_cast<const uint8_t*>(src);

    if (d < s) {
        for (size_t i = 0; i < n; i++) {
            d[i] = s[i];
        }
    } else if (d > s) {
        for (size_t i = n; i > 0; i--) {
            d[i - 1] = s[i - 1];
        }
    }

    return dest;
}

int memcmp(const void* s1, const void* s2, size_t n)
{
    const auto* p1 = reinterpret_cast<const uint8_t*>(s1);
    const auto* p2 = reinterpret_cast<const uint8_t*>(s2);
    while (n--) {
        if (*p1 != *p2)
            return *p1 - *p2;

        p1++;
        p2++;
    }
    return 0;
}
}
