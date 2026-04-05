#include "subsystems/vmm/vmm.h"
#include "limine/requests.h"
#include "mem/memory.h"
#include "systems.h"

#include <kt/intrin.h>

namespace kt_kernel
{
    void vmm::initialize()
    {
        hhdm_offset = limine::hhdm_request.response->offset;
        pml4_physical = read_cr3() & static_cast<uint64_t>(page_flags::address_mask);
    }

    uint64_t* vmm::get_pte(uint64_t virt, bool create)
    {
        uint64_t* table = reinterpret_cast<uint64_t*>(pml4_physical + hhdm_offset);

        for (int level = 3; level > 0; level--) {
            uint64_t index = (virt >> (12 + 9 * level)) & 0x1FF;

            if (!(table[index] & static_cast<uint64_t>(page_flags::present))) {
                if (!create)
                    return nullptr;

                uint64_t frame = g_kernel_context->pmm->allocate_frame();
                if (frame == 0)
                    return nullptr;

                auto* page = reinterpret_cast<uint64_t*>(frame + hhdm_offset);
                memset(page, 0, 512 * sizeof(uint64_t));

                table[index] = frame | static_cast<uint64_t>(page_flags::present | page_flags::writable);
            }
            table = reinterpret_cast<uint64_t*>((table[index] & static_cast<uint64_t>(page_flags::address_mask)) +
                                                 hhdm_offset);
        }

        return &table[(virt >> 12) & 0x1FF];
    }

    bool vmm::map(uint64_t virt, uint64_t phys, uint64_t flags)
    {
        uint64_t* pte = get_pte(virt, true);
        if (!pte)
            return false;

        *pte = (phys & static_cast<uint64_t>(page_flags::address_mask)) | flags |
               static_cast<uint64_t>(page_flags::present);
        return true;
    }

    void vmm::unmap(uint64_t virt)
    {
        uint64_t* pte = get_pte(virt, false);
        if (!pte)
            return;

        *pte = 0;
        invlpg(virt);
    }

    uint64_t vmm::virtual_to_physical(uint64_t virt)
    {
        uint64_t* pte = get_pte(virt, false);
        if (!pte || !(*pte & static_cast<uint64_t>(page_flags::present)))
            return 0;

        return (*pte & static_cast<uint64_t>(page_flags::address_mask)) | (virt & 0x1FF);
    }

    uint64_t vmm::physical_to_virtual(uint64_t phys)
    {
        return phys + hhdm_offset;
    }

    void vmm::activate()
    {
        write_cr3(pml4_physical);
    }
} // namespace kt_kernel
