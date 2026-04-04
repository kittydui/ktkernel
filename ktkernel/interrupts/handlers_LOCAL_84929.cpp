#include "interrupts.h"
#include "subsystems/console/logging.h"

namespace KtKernel
{
    void Int3Handler(uint8_t interruptIndex, uint64_t errorCode)
    {
        KtCore::KPrint("bp hit (interrupt {})", interruptIndex);
    }

    void GPHandler(uint8_t interruptIndex, uint64_t errorCode)
    {
        KtCore::KPrint("General Protection Fault");
    }
} // namespace KtKernel
