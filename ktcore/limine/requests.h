#pragma once

#include "limine/limine.h"

extern "C"
{
extern volatile uint64_t g_limineBaseRevision[];

extern volatile limine_rsdp_request g_rsdpRequest;
extern volatile limine_framebuffer_request g_framebufferRequest;
extern volatile limine_memmap_request g_memmapRequest;
extern volatile limine_hhdm_request g_hhdmRequest;
extern volatile limine_module_request g_moduleRequest;

extern volatile uint64_t g_limineRequestsStartMarker[];
extern volatile uint64_t g_limineRequestsEndMarker[];
}
