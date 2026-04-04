#include "interrupts/interrupts.h"
#include "subsystems/console/logging.h"

namespace KtKernel
{
    [[noreturn]] void ExceptionHandler()
    {
        KtCore::Panic("Exception Handler called!");

        asm volatile("cli; hlt");
    }
} // namespace KtKernel
