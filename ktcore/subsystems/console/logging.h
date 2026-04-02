#pragma once
#include "systems.h"

namespace KtCore
{
    template <typename... Args> inline void KPrint(const char* str, Args... args)
    {
        g_kernelContext->m_console->print("[ktkernel] ");
        g_kernelContext->m_console->printf(str, args...);
        g_kernelContext->m_console->print("\n");
    }

    void Log(const char* str);
    void LogPrefix(const char* prefix, const char* str);

    template <typename... Args> inline void Panic(const char* str, Args... args)
    {
        g_kernelContext->m_console->print("========== PANIC! ==========", CC_WHITE, CC_RED);
        g_kernelContext->m_console->print("\n");
        g_kernelContext->m_console->printf(CC_WHITE, CC_RED, str, args...);
        g_kernelContext->m_console->print("\n");
        g_kernelContext->m_console->print("========== PANIC! ==========", CC_WHITE, CC_RED);

        while (true)
            asm volatile("hlt");
    }
} // namespace KtCore
