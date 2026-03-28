#pragma once
#include <cstddef>

extern "C"
{
auto memcpy(void* dest, const void* src, size_t n) -> void*;
auto memset(void* s, int c, size_t n) -> void*;
auto memmove(void* dest, const void* src, size_t n) -> void*;
auto memcmp(const void* s1, const void* s2, size_t n) -> int;
}
