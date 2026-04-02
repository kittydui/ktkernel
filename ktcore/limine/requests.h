#pragma once

#include "limine/limine.h"

namespace Limine
{
    extern "C"
    {
        extern volatile uint64_t limineBaseRevision[];

        extern volatile limine_rsdp_request rsdpRequest;
        extern volatile limine_framebuffer_request framebufferRequest;
        extern volatile limine_memmap_request memmapRequest;
        extern volatile limine_hhdm_request hhdmRequest;
        extern volatile limine_module_request moduleRequest;
        extern volatile limine_mp_request mpRequest;

        extern volatile uint64_t limineRequestsStartMarker[];
        extern volatile uint64_t limineRequestsEndMarker[];
    }
} // namespace Limine
