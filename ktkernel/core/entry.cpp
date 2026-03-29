#include "core/init.h"
#include "cpu/utilities.h"
#include "systems.h"

extern "C" [[noreturn]] void KtMain()
{
    KtKernel::InitializeSubsystems();
    KtCore::KPrint("Mreow");
    KtCore::Halt();
}
