#include "core/init.h"
#include "cpu/utilities.h"
#include "cxxabi.h"
#include "subsystems/console/logging.h"
#include "systems.h"

#if defined(QEMU)
#include "devices/qemu/debug_port.h"
#endif

#include "modules/loader.h"

#include <kt/types.h>

extern "C" [[noreturn]] void KtMain()
{
    KtKernel::InitializeSubsystems();

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
