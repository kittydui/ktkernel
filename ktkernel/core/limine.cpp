#include "limine/requests.h"

extern "C"
{
__attribute__((used, section(".limine_requests"))) volatile uint64_t g_limineBaseRevision[] = LIMINE_BASE_REVISION(6);

__attribute__((used, section(".limine_requests"))) volatile limine_framebuffer_request g_framebufferRequest = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
};

__attribute__((used, section(".limine_requests"))) volatile limine_rsdp_request g_rsdpRequest = {
    .id = LIMINE_RSDP_REQUEST_ID,
    .revision = 0,
};

__attribute__((used, section(".limine_requests"))) volatile limine_memmap_request g_memmapRequest = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0,
};

__attribute__((used, section(".limine_requests"))) volatile limine_hhdm_request g_hhdmRequest = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0,
};

__attribute__((used, section(".limine_requests"))) volatile limine_module_request g_moduleRequest = {
    .id = LIMINE_MODULE_REQUEST_ID,
    .revision = 0,
};

__attribute__((used, section(".limine_requests_start"))) volatile uint64_t g_limineRequestsStartMarker[] =
    LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end"))) volatile uint64_t g_limineRequestsEndMarker[] =
    LIMINE_REQUESTS_END_MARKER;
}
