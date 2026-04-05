#pragma once
#include <cstdint>

namespace kt_kernel
{
    struct rsdp
    {
        char signature[8];
        uint8_t checksum;
        char oem_id[6];
        uint8_t revision;
        uint32_t rsdt_address;
        uint32_t length;
        uint64_t xsdt_address;
        uint8_t extended_checksum;
        uint8_t reserved[3];

        bool initialize();
    } __attribute__((packed));
} // namespace kt_kernel
