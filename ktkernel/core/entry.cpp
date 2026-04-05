#include "core/init.h"
#include "interrupts/interrupts.h"
#include "modules/loader.h"
#include "subsystems/console/logging.h"
#include "utilities/gdt.h"

#include <kt/module.h>
#include <kt/types.h>

namespace kt_kernel
{
    extern "C" [[noreturn]] void kt_main()
    {
        initialize_subsystems();
        setup_gdt();
        setup_idt();

        kt_module* mod = get_module("cmos_driver");

        mod->dispatch_functions.read(mod, g_kernel_context->current_time, sizeof(kt_date_time));

        auto* time = g_kernel_context->current_time;
        print("time: {}:{}:{} {}/{}/{}", time->hour, time->minute, time->second, time->day, time->month, time->year);

        while (true)
            asm volatile("hlt");
    }
} // namespace kt_kernel
