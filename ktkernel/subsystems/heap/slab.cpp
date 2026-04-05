#include "subsystems/heap/slab.h"
#include "limine/requests.h"
#include "mem/memory.h"
#include "systems.h"

namespace kt_kernel
{
    void slab_allocator::initialize()
    {
        hhdm_offset = limine::hhdm_request.response->offset;

        for (size_t i = 0; i < 6; i++) {
            caches[i].object_size = class_sz[i];
            caches[i].slabs = nullptr;
        }
    }

    int slab_allocator::find_class(size_t size)
    {
        for (size_t i = 0; i < 6; i++)
            if (size <= class_sz[i])
                return static_cast<int>(i);

        return -1;
    }

    slab_allocator::slab_header* slab_allocator::create_slab(size_t size)
    {
        uint64_t frame = g_kernel_context->pmm->allocate_frame();
        if (!frame)
            return nullptr;

        auto* slab = reinterpret_cast<slab_header*>(frame + hhdm_offset);
        slab->magic = magic_number;
        slab->object_size = size;
        slab->used_count = 0;
        slab->next = nullptr;
        slab->free_list = nullptr;

        auto data_start = reinterpret_cast<uint64_t>(slab) + sizeof(slab_header);
        size_t available = page_size - sizeof(slab_header);
        slab->total_count = available / size;

        for (size_t i = 0; i < slab->total_count; i++) {
            auto* object = reinterpret_cast<void**>(data_start + i * size);
            *object = slab->free_list;
            slab->free_list = object;
        }

        return slab;
    }

    void* slab_allocator::allocate_from_slab(cache& c)
    {
        slab_header* slab = c.slabs;
        while (slab && !slab->free_list)
            slab = slab->next;

        if (!slab) {
            slab = create_slab(c.object_size);
            if (!slab)
                return nullptr;

            slab->next = c.slabs;
            c.slabs = slab;
        }

        void* object = slab->free_list;
        slab->free_list = *reinterpret_cast<void**>(object);
        slab->used_count++;
        return object;
    }

    void* slab_allocator::allocate_large(size_t size)
    {
        size_t total = size + sizeof(large_header);
        size_t pages = (total + page_size - 1) / page_size;

        uint64_t frame = g_kernel_context->pmm->allocate_contiguous(pages);
        if (!frame)
            return nullptr;

        auto* header = reinterpret_cast<large_header*>(frame + hhdm_offset);
        header->pages = pages;

        return header + 1;
    }

    void* slab_allocator::allocate(size_t size)
    {
        if (size == 0)
            return nullptr;

        int idx = find_class(size);
        if (idx >= 0)
            return allocate_from_slab(caches[idx]);

        return allocate_large(size);
    }

    void slab_allocator::free(void* ptr)
    {
        if (!ptr)
            return;

        auto page = reinterpret_cast<uint64_t>(ptr) & ~(page_size - 1);
        auto* slab = reinterpret_cast<slab_header*>(page);

        if (slab->magic == magic_number) {
            *reinterpret_cast<void**>(ptr) = slab->free_list;
            slab->free_list = ptr;
            slab->used_count--;
            return;
        }

        auto* header = reinterpret_cast<large_header*>(ptr) - 1;
        uint64_t phys = reinterpret_cast<uint64_t>(header) - hhdm_offset;
        g_kernel_context->pmm->free_contiguous(phys, header->pages);
    }
} // namespace kt_kernel
