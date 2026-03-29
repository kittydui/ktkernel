#pragma once
#include "limine/limine.h"
#include "subsystems/console/format.h"
#include "subsystems/console/psf2.h"

enum ConsoleColor : uint32_t {
    // ARGB
    CC_BLACK = 0xFF000000,
    CC_WHITE = 0xFFFFFFFF,
    CC_RED = 0xFFFF0000,
    CC_GREEN = 0xFF00FF00,
    CC_BLUE = 0xFF0000FF
};

namespace KtCore {
class FramebufferConsole {
public:
    bool initialize();
    void shutdown();

    void putChar(char c, uint32_t foreground, uint32_t background);
    void print(const char* str, uint32_t foreground = CC_WHITE, uint32_t background = CC_BLACK);
    void clear(uint32_t color = CC_BLACK);

    template <typename... Args> void printf(const char* format, Args... args)
    {
        printf(CC_WHITE, CC_BLACK, format, args...);
    }

    template <typename... Args> void printf(uint32_t fg, uint32_t bg, const char* format, Args... args)
    {
        Internal::FormatBuffer buf{ m_fmtBuffer, sizeof(m_fmtBuffer), 0 };
        Internal::FormatImpl(buf, format, args...);
        buf.terminate();
        print(m_fmtBuffer, fg, bg);
    }

private:
    void scroll();
    void drawCharacter(int x, int y, char c, uint32_t foreground, uint32_t background);

    limine_framebuffer* m_framebuffer = nullptr;
    PSF2Header* m_font = nullptr;
    uint32_t m_cursorX = 0;
    uint32_t m_cursorY = 0;
    char m_fmtBuffer[1024]{};
};

} // namespace KtCore
