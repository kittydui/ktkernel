#pragma once
#include <cstddef>
#include <cstdint>

namespace kt_kernel
{
    class slab_allocator
    {
    public:
        void initialize();
        void* allocate(size_t size);
        void free(void* ptr);

    private:
        static constexpr uint64_t magic_number = 0x534C4142;
        static constexpr uint64_t page_size = 4096;
        static constexpr size_t class_sz[6] = { 32, 64, 128, 256, 512, 1024 };

        struct slab_header
        {
            uint64_t magic;
            void* free_list;
            slab_header* next;
            size_t object_size;
            size_t used_count;
            size_t total_count;
        };

        struct large_header
        {
            size_t pages;
            size_t padding_0;
        };

        struct cache
        {
            slab_header* slabs = nullptr;
            size_t object_size = 0;
        };

        cache caches[6];
        uint64_t hhdm_offset = 0;

        int find_class(size_t size);
        slab_header* create_slab(size_t size);
        void* allocate_from_slab(cache& c);
        void* allocate_large(size_t size);
    };
} // namespace kt_kernel
