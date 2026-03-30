#include "core/init.h"
#include "cpu/utilities.h"
#include "limine/requests.h"
#include "systems.h"

#include "subsystems/console/logging.h"

namespace KtKernel
{
    void InitializeSubsystems()
    {
        if (!CheckLimineFeatures())
            halt();

        static KtCore::KernelContext ctx;
        KtCore::g_kernelContext = &ctx;

        static KtCore::PMM pmm;
        ctx.m_pmm = &pmm;
        ctx.m_pmm->initialize();

        static KtCore::VMM vmm;
        ctx.m_vmm = &vmm;
        ctx.m_vmm->initialize();

        static KtCore::SlabAllocator allocator;
        ctx.m_allocator = &allocator;
        ctx.m_allocator->initialize();

        // Initialize everything else, do NOT initialize anything before the PMM, VMM and Allocator.

        static KtCore::Console console;
        ctx.m_console = &console;
        ctx.m_console->initialize();

        static KtCore::SerialPort com1_port;
        com1_port.initialize(KtCore::COM1_PORT);
        ctx.m_console->attachSerialPort(&com1_port);

        KtCore::KPrint("{}MB Available\n", KtCore::BytesToMB(ctx.m_pmm->getTotalMemory()));

        ctx.m_rsdp = reinterpret_cast<KtCore::RSDP*>(Limine::rsdpRequest.response->address);
        ctx.m_rsdp->initialize();
    }

    bool CheckLimineFeatures()
    {
        if (Limine::moduleRequest.response == nullptr || Limine::moduleRequest.response->module_count < 1)
            return false;

        if (Limine::framebufferRequest.response == nullptr ||
            Limine::framebufferRequest.response->framebuffer_count < 1)
            return false;

        if (Limine::rsdpRequest.response == nullptr || Limine::rsdpRequest.response->address == nullptr)
            return false;

        if (Limine::memmapRequest.response == nullptr || Limine::memmapRequest.response->entry_count < 1)
            return false;

        if (Limine::hhdmRequest.response == nullptr || Limine::hhdmRequest.response->offset == 0)
            return false;

        return true;
    }
} // namespace KtKernel
