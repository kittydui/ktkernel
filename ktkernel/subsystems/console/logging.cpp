#include "subsystems/console/logging.h"
#include "symbol.h"

#include <cstdarg>

namespace kt_kernel
{
    void log(const char* str)
    {
        g_kernel_context->console->print(str);
        g_kernel_context->console->print("\n");
    }

    void log_prefix(const char* prefix, const char* str)
    {
        g_kernel_context->console->print(prefix);
        g_kernel_context->console->print(str);
        g_kernel_context->console->print("\n");
    }

    void log_fmt(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        g_kernel_context->console->print("[ktkernel] ");
        g_kernel_context->console->v_printf(fmt, args);
        g_kernel_context->console->print("\n");
        va_end(args);
    }

    kt_export_symbol(log_fmt);
    kt_export_symbol(log);
    kt_export_symbol(log_prefix);
} // namespace kt_kernel
