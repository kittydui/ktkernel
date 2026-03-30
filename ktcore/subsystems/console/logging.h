#pragma once
#include "systems.h"

namespace KtCore
{
template <typename... Args> inline void KPrint(const char* str, Args... args)
{
    g_kernelContext->m_console->printf(str, args...);
}
} // namespace KtCore
