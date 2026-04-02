#pragma once
#include <cstddef>

extern "C"
{
    void* KMalloc(size_t size);
    void KFree(void* ptr);
    void Log(const char* str);
    void LogPrefix(const char* prefix, const char* str);
    void LogFmt(const char* fmt, ...);
}
