#pragma once

#include "limine/limine.h"

namespace limine
{
    extern "C"
    {
        extern volatile uint64_t limine_base_revision[];

        extern volatile limine_rsdp_request rsdp_request;
        extern volatile limine_framebuffer_request framebuffer_request;
        extern volatile limine_memmap_request memmap_request;
        extern volatile limine_hhdm_request hhdm_request;
        extern volatile limine_module_request module_request;
        extern volatile limine_mp_request mp_request;

        extern volatile uint64_t limine_requests_start_marker[];
        extern volatile uint64_t limine_requests_end_marker[];
    }
} // namespace limine
