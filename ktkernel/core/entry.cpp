#include "limine/requests.h"

extern "C" [[noreturn]] auto KtMain() -> void
{
    while (true) {
        __asm__("hlt");
    }
}
