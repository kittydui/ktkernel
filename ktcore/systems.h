#pragma once
#include "subsystems/console/console.h"

namespace KtCore {

struct KernelContext {
    FramebufferConsole* m_console = nullptr;
};

inline KernelContext* g_KernelContext = nullptr;

template <typename... Args> inline void KPrint(const char* str, Args... args)
{
    g_KernelContext->m_console->printf(str, args...);
}

} // namespace KtCore
