#include "interrupts/handlers.h"
#include "subsystems/console/logging.h"

namespace kt_kernel
{
    void int3_handler(uint8_t interrupt_index, uint64_t)
    {
        print("bp hit (interrupt {})", interrupt_index);
    }

    void gp_handler(uint8_t, uint64_t)
    {
        print("General Protection Fault");
    }
} // namespace kt_kernel
