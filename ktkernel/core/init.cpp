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
        KtCore::Halt();

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

    static KtCore::FramebufferConsole console;
    ctx.m_console = &console;
    ctx.m_console->initialize();

    KtCore::KPrint("{}MB Available\n", KtCore::BytesToMB(ctx.m_pmm->getTotalMemory()));

    ctx.m_rsdp = reinterpret_cast<KtCore::RSDP*>(g_rsdpRequest.response->address);
    ctx.m_rsdp->initialize();
}

bool CheckLimineFeatures()
{
    if (g_moduleRequest.response == nullptr || g_moduleRequest.response->module_count < 1)
        return false;

    if (g_framebufferRequest.response == nullptr || g_framebufferRequest.response->framebuffer_count < 1)
        return false;

    if (g_rsdpRequest.response == nullptr || g_rsdpRequest.response->address == nullptr)
        return false;

    if (g_memmapRequest.response == nullptr || g_memmapRequest.response->entry_count < 1)
        return false;

    if (g_hhdmRequest.response == nullptr || g_hhdmRequest.response->offset == 0)
        return false;

    return true;
}
} // namespace KtKernel
