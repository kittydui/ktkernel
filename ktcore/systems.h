#pragma once
#include "subsystems/acpi/rsdp.h"
#include "subsystems/console/console.h"

namespace KtCore
{
struct KernelContext
{
    FramebufferConsole* m_console = nullptr;
    RSDP* m_rsdp = nullptr;
};

inline KernelContext* g_kernelContext = nullptr;

template <typename... Args> inline void KPrint(const char* str, Args... args)
{
    g_kernelContext->m_console->printf(str, args...);
}
} // namespace KtCore
