#include "limine/requests.h"

namespace Limine
{
    extern "C"
    {
        __attribute__((used, section(".limine_requests"))) volatile uint64_t limineBaseRevision[] =
            LIMINE_BASE_REVISION(6);

        __attribute__((used, section(".limine_requests"))) volatile limine_framebuffer_request framebufferRequest = {
            .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
            .revision = 0,
        };

        __attribute__((used, section(".limine_requests"))) volatile limine_rsdp_request rsdpRequest = {
            .id = LIMINE_RSDP_REQUEST_ID,
            .revision = 0,
        };

        __attribute__((used, section(".limine_requests"))) volatile limine_memmap_request memmapRequest = {
            .id = LIMINE_MEMMAP_REQUEST_ID,
            .revision = 0,
        };

        __attribute__((used, section(".limine_requests"))) volatile limine_hhdm_request hhdmRequest = {
            .id = LIMINE_HHDM_REQUEST_ID,
            .revision = 0,
        };

        __attribute__((used, section(".limine_requests"))) volatile limine_module_request moduleRequest = {
            .id = LIMINE_MODULE_REQUEST_ID,
            .revision = 0,
        };

        __attribute__((used, section(".limine_requests"))) volatile limine_mp_request mpRequest = {
            .id = LIMINE_MP_REQUEST_ID,
            .revision = 0,
        };

        __attribute__((used, section(".limine_requests_start"))) volatile uint64_t limineRequestsStartMarker[] =
            LIMINE_REQUESTS_START_MARKER;

        __attribute__((used, section(".limine_requests_end"))) volatile uint64_t limineRequestsEndMarker[] =
            LIMINE_REQUESTS_END_MARKER;
    }
} // namespace Limine
