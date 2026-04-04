#include "interrupts.h"
#include "subsystems/console/logging.h"

namespace KtKernel
{
    void Int3Handler(uint8_t interruptIndex, uint64_t errorCode)
    {
<<<<<<< HEAD
        KtCore::KPrint("bp hit (interrupt {})", interruptIndex);
=======
        KtCore::KPrint("bp hit", interruptIndex);
>>>>>>> 5811a2c0b9dc59e2ebb722da6f281bbb8457164b
    }

    void GPHandler(uint8_t interruptIndex, uint64_t errorCode)
    {
        KtCore::KPrint("General Protection Fault");
    }
} // namespace KtKernel
