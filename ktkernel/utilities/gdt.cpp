#include "utilities/gdt.h"
#include <kt/intrin.h>

namespace kt_kernel
{
    bool setup_gdt()
    {
        gdt_ptr gdtr = { sizeof(gdt) - 1, reinterpret_cast<uint64_t>(gdt) };
        load_gdt(gdtr);

        asm volatile("mov %0, %%ds\n"
                     "mov %0, %%es\n"
                     "mov %0, %%fs\n"
                     "mov %0, %%gs\n"
                     "mov %0, %%ss\n"
                     "pushq %1\n"
                     "leaq 1f(%%rip), %%rax\n"
                     "pushq %%rax\n"
                     "lretq\n"
                     "1:\n"
                     :
                     : "r"((uint64_t)kernel_data_selector), "i"(kernel_code_selector)
                     : "rax", "memory");
        return true;
    }
} // namespace kt_kernel
