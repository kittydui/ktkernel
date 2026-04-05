#include "core/init.h"
#include "limine/requests.h"
#include "modules/loader.h"
#include "subsystems/console/logging.h"
#include "utilities/tar.h"

#include <kt/intrin.h>
#include <kt/types.h>

namespace kt_kernel
{
    void initialize_subsystems()
    {
        if (!check_limine_requests())
            halt();

        static kernel_context ctx;
        g_kernel_context = &ctx;

        static pmm physical_mm;
        ctx.pmm = &physical_mm;
        ctx.pmm->initialize();

        static vmm virtual_mm;
        ctx.vmm = &virtual_mm;
        ctx.vmm->initialize();

        static slab_allocator allocator;
        ctx.allocator = &allocator;
        ctx.allocator->initialize();

        auto* system_tar = limine::module_request.response->modules[0];

        tar_archive systems_archive;
        systems_archive.open(system_tar->address, system_tar->size);

        auto font = systems_archive.read_file("fonts/Lat2-Terminus16.psfu");

        static console kernel_console;
        ctx.console = &kernel_console;
        ctx.console->initialize((void*)font.data);

        static serial_port com1;
        com1.initialize(com1_port);
        ctx.console->attach_serial_port(&com1);

        ctx.current_time = reinterpret_cast<kt_date_time*>(kmalloc(sizeof(kt_date_time)));

        print("{}MB Available", bytes_to_mb(ctx.pmm->get_total_memory()));

        ctx.rsdp = reinterpret_cast<rsdp*>(limine::rsdp_request.response->address);
        ctx.rsdp->initialize();

        load_core_modules();
    }

    bool check_limine_requests()
    {
        if (limine::module_request.response == nullptr || limine::module_request.response->module_count < 1)
            return false;

        if (limine::framebuffer_request.response == nullptr ||
            limine::framebuffer_request.response->framebuffer_count < 1)
            return false;

        if (limine::rsdp_request.response == nullptr || limine::rsdp_request.response->address == nullptr)
            return false;

        if (limine::memmap_request.response == nullptr || limine::memmap_request.response->entry_count < 1)
            return false;

        if (limine::hhdm_request.response == nullptr || limine::hhdm_request.response->offset == 0)
            return false;

        return true;
    }
} // namespace kt_kernel
