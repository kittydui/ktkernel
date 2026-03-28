#include "console/console.h"
#include "cpu/utilities.h"
#include "limine/requests.h"

extern "C" [[noreturn]] void KtMain()
{
    if (g_moduleRequest.response == 0 || g_moduleRequest.response->module_count < 1)
        KtCore::Halt();

    if (g_framebufferRequest.response == 0 || g_framebufferRequest.response->framebuffer_count < 1)
        KtCore::Halt();

    limine_framebuffer* framebuffer = g_framebufferRequest.response->framebuffers[0];
    limine_file* font_file = g_moduleRequest.response->modules[0];

    static KtCore::FramebufferConsole fb_console(framebuffer, font_file->address);
    KtCore::g_KConsole = &fb_console;

    ClearConsole();
    Print("Mreow :3");

    KtCore::Halt();
}
