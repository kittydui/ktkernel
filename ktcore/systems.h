#pragma once
#include "subsystems/acpi/rsdp.h"
#include "subsystems/console/console.h"
#include "subsystems/heap/slab.h"
#include "subsystems/pmm/pmm.h"
#include "subsystems/vmm/vmm.h"

namespace KtCore
{
    struct KernelContext
    {
        Console* m_console = nullptr;
        RSDP* m_rsdp = nullptr;
        PMM* m_pmm = nullptr;
        VMM* m_vmm = nullptr;
        SlabAllocator* m_allocator = nullptr;
    };

    inline KernelContext* g_kernelContext = nullptr;
} // namespace KtCore
