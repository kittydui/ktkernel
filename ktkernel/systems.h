#pragma once
#include "subsystems/acpi/rsdp.h"
#include "subsystems/console/console.h"
#include "subsystems/heap/slab.h"
#include "subsystems/pmm/pmm.h"
#include "subsystems/vmm/vmm.h"

#include <kt/types.h>

namespace kt_kernel
{
    struct kernel_context
    {
        console* console = nullptr;
        rsdp* rsdp = nullptr;
        pmm* pmm = nullptr;
        vmm* vmm = nullptr;
        slab_allocator* allocator = nullptr;
        kt_date_time* current_time = nullptr;
    };

    inline kernel_context* g_kernel_context = nullptr;
} // namespace kt_kernel
