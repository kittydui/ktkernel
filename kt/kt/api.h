#pragma once
#include <cstddef>

extern "C"
{
    void* kmalloc(size_t size);
    void kfree(void* ptr);
    void log(const char* str);
    void log_prefix(const char* prefix, const char* str);
    void log_fmt(const char* fmt, ...);
}
