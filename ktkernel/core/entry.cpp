#include "console/console.h"
#include "limine/requests.h"

// import default console font:

extern "C"
{
extern uint8_t _binary_fonts_Lat2_Terminus16_psfu_start[];
extern uint8_t _binary_fonts_Lat2_Terminus16_psfu_size[];
extern uint8_t _binary_fonts_Lat2_Terminus16_psfu_end[];
}

extern "C" [[noreturn]] void KtMain()
{
    if (g_framebufferRequest.response == 0 || g_framebufferRequest.response->framebuffer_count < 1)
        while (true)
            __asm__("hlt");

    struct limine_framebuffer* framebuffer = g_framebufferRequest.response->framebuffers[0];

    static KtCore::FramebufferConsole fb_console(framebuffer, _binary_fonts_Lat2_Terminus16_psfu_start);
    KtCore::g_KConsole = &fb_console;

    KtCore::g_KConsole->clear();
    KtCore::g_KConsole->print("mreow");

    while (true)
        __asm__("hlt");
}
