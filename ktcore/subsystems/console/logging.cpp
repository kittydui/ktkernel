#include "subsystems/console/logging.h"
#include "symbol.h"

#include <cstdarg>

namespace KtCore
{
    void Log(const char* str)
    {
        g_kernelContext->m_console->print(str);
        g_kernelContext->m_console->print("\n");
    }

    void LogPrefix(const char* prefix, const char* str)
    {
        g_kernelContext->m_console->print(prefix);
        Log(str);
    }

    void LogFmt(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);

        g_kernelContext->m_console->print("[ktkernel] ");
        g_kernelContext->m_console->vprintf(fmt, args);
        g_kernelContext->m_console->print("\n");

        va_end(args);
    }

    KtExportSymbol(LogFmt);
    KtExportSymbol(Log);
    KtExportSymbol(LogPrefix);
} // namespace KtCore
