#include "subsystems/vmm/vmm.h"
#include "cpu/utilities.h"
#include "limine/requests.h"
#include "mem/memory.h"
#include "systems.h"

namespace KtCore
{
    void VMM::initialize()
    {
        m_hhdmOffset = Limine::hhdmRequest.response->offset;
        m_pml4Physical = readCr3() & PageFlags::ADDRESS_MASK;
    }

    uint64_t* VMM::getPte(uint64_t virt, bool create)
    {
        uint64_t* table = reinterpret_cast<uint64_t*>(m_pml4Physical + m_hhdmOffset);

        for (int level = 3; level > 0; level--) {
            uint64_t index = (virt >> (12 + 9 * level)) & 0x1FF;

            if (!(table[index] & PageFlags::PRESENT)) {
                if (!create)
                    return nullptr;

                uint64_t frame = g_kernelContext->m_pmm->allocateFrame();
                if (frame == 0)
                    return nullptr;

                auto* page = reinterpret_cast<uint64_t*>(frame + m_hhdmOffset);
                memset(page, 0, 512);

                table[index] = frame | PageFlags::PRESENT | PageFlags::WRITABLE;
            }
            table = reinterpret_cast<uint64_t*>((table[index] & PageFlags::ADDRESS_MASK) + m_hhdmOffset);
        }

        return &table[(virt >> 12) & 0x1FF];
    }

    bool VMM::map(uint64_t virt, uint64_t phys, uint64_t flags)
    {
        uint64_t* pte = getPte(virt, true);
        if (!pte)
            return false;

        *pte = (phys & PageFlags::ADDRESS_MASK) | flags | PageFlags::PRESENT;
        return true;
    }

    void VMM::unMap(uint64_t virt)
    {
        uint64_t* pte = getPte(virt, false);
        if (!pte)
            return;

        *pte = 0;
        invlpg(virt);
    }

    uint64_t VMM::virtualToPhysical(uint64_t virt)
    {
        uint64_t* pte = getPte(virt, false);
        if (!pte || !(*pte & PageFlags::PRESENT))
            return 0;

        return (*pte & PageFlags::ADDRESS_MASK) | (virt & 0x1FF);
    }

    uint64_t VMM::physicalToVirtual(uint64_t phys)
    {
        return phys + m_hhdmOffset;
    }

    void VMM::activate()
    {
        writeCr3(m_pml4Physical);
    }
} // namespace KtCore
