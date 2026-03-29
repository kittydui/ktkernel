#pragma once

namespace KtCore
{
[[noreturn]] inline void Halt()
{
    while (true)
        __asm__("hlt");
}
} // namespace KtCore
