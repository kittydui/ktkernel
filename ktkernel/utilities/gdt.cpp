#include "utilities/gdt.h"
#include <kt/intrin.h>

namespace KtKernel
{
    bool SetupGdt()
    {
        GDTR gdtr = { sizeof(GDT) - 1, GDT };
        loadGdt(gdtr);

        asm volatile("mov %0, %%ds\n"
                     "mov %0, %%es\n"
                     "mov %0, %%fs\n"
                     "mov %0, %%gs\n"
                     "mov %0, %%ss\n"
                     // far return to reload CS
                     "pushq %1\n"
                     "leaq 1f(%%rip), %%rax\n"
                     "pushq %%rax\n"
                     "lretq\n"
                     "1:\n"
                     :
                     : "r"((uint64_t)KERNEL_DATA_SELECTOR), "i"(KERNEL_CODE_SELECTOR)
                     : "rax", "memory");
        return true;
    }
} // namespace KtKernel
