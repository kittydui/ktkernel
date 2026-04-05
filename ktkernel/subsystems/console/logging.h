#pragma once
#include "subsystems/console/console.h"
#include "systems.h"

namespace kt_kernel
{
    template <typename... Args> inline void print(const char* str, Args... args)
    {
        g_kernel_context->console->print("[ktkernel] ");
        g_kernel_context->console->printf(str, args...);
        g_kernel_context->console->print("\n");
    }

    void log(const char* str);
    void log_prefix(const char* prefix, const char* str);

    template <typename... Args> [[noreturn]] inline void panic(const char* str, Args... args)
    {
        g_kernel_context->console->print("========== PANIC! ==========",
                                         static_cast<uint32_t>(console_color::white),
                                         static_cast<uint32_t>(console_color::red));
        g_kernel_context->console->print("\n");
        g_kernel_context->console->printf(
            static_cast<uint32_t>(console_color::white), static_cast<uint32_t>(console_color::red), str, args...);
        g_kernel_context->console->print("\n");
        g_kernel_context->console->print("========== PANIC! ==========",
                                         static_cast<uint32_t>(console_color::white),
                                         static_cast<uint32_t>(console_color::red));

        while (true)
            asm volatile("cli; hlt");
    }
} // namespace kt_kernel
