#pragma once
#include "console/psf2.h"
#include "limine/limine.h"

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
    FramebufferConsole(limine_framebuffer* framebuffer, void* fontData);

    void putChar(char c, uint32_t foreground, uint32_t background);
    void print(const char* str, uint32_t foreground = CC_WHITE, uint32_t background = CC_BLACK);
    void clear(uint32_t color = CC_BLACK);

private:
    void scroll();
    void drawCharacter(int x, int y, char c, uint32_t foreground, uint32_t background);

    limine_framebuffer* m_framebuffer;
    PSF2Header* m_font;
    uint32_t m_cursorX = 0;
    uint32_t m_cursorY = 0;
};

inline FramebufferConsole* g_KConsole;
} // namespace KtCore
  //
#define Print(msg) KtCore::g_KConsole->print(msg)
#define ClearConsole() KtCore::g_KConsole->clear()
