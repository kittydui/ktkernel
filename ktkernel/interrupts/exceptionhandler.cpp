#include "interrupts/handlers.h"
#include "subsystems/console/logging.h"

namespace kt_kernel
{
    [[noreturn]] void exception_handler()
    {
        panic("Exception Handler called!");
    }
} // namespace kt_kernel
