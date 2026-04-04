#include "core/init.h"
#include "modules/loader.h"
#include "subsystems/console/logging.h"
#include "systems.h"

#include <kt/types.h>

#if defined(QEMU)
#include "devices/qemu/debug_port.h"
#endif

#include "interrupts/interrupts.h"
#include "utilities/gdt.h"

namespace KtKernel
{
    extern "C" [[noreturn]] void KtMain()
    {
        SetupGdt();
        InitializeSubsystems();
        SetupIdt(); // setup interrupts after initializing the console
                    // so we can show that an interrupt was called.

        KtModule* mod = KtKernel::GetModule("cmos_driver");

        mod->m_dispatchFunctions.Read(mod, KtCore::g_kernelContext->m_currentTime, sizeof(KtDateTime));

        auto* time = KtCore::g_kernelContext->m_currentTime;
        KtCore::KPrint("time: {}:{}:{} {}/{}/{}",
                       time->m_Hour,
                       time->m_Minute,
                       time->m_Second,
                       time->m_Day,
                       time->m_Month,
                       time->m_Year);

        while (true)
            ;

#if defined(QEMU)
        KtCore::Qemu::Exit(0x10);
#endif
    }
} // namespace KtKernel
