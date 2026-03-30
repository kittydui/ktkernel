#include "core/init.h"
#include "cpu/utilities.h"
#include "cxxabi.h"
#include "systems.h"

extern "C" [[noreturn]] void KtMain()
{
    // initialize_ctors();
    KtKernel::InitializeSubsystems();
    halt();
}
