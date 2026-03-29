#include "core/init.h"
#include "cpu/utilities.h"
#include "limine/requests.h"
#include "systems.h"

namespace KtKernel {
void InitializeSubsystems()
{
    if (!CheckLimineFeatures())
        KtCore::Halt();

    static KtCore::KernelContext ctx;
    KtCore::g_KernelContext = &ctx;

    static KtCore::FramebufferConsole console;
    ctx.m_console = &console;
    ctx.m_console->initialize();
}

bool CheckLimineFeatures()
{
    if (g_moduleRequest.response == nullptr || g_moduleRequest.response->module_count < 1)
        return false;

    if (g_framebufferRequest.response == nullptr || g_framebufferRequest.response->framebuffer_count < 1)
        return false;

    return true;
}
} // namespace KtKernel
