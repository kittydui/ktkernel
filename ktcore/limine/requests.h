#pragma once

#include "limine/limine.h"

extern "C"
{
extern volatile uint64_t g_limineBaseRevision[];

extern volatile limine_rsdp_request g_rsdpRequest;
extern volatile limine_framebuffer_request g_framebufferRequest;

extern volatile uint64_t g_limineRequestsBeginMarker[];
extern volatile uint64_t g_limineRequestsEndMarker[];
}
