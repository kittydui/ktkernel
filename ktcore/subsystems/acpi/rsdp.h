#pragma once
#include <cstdint>

namespace KtCore
{
    struct RSDP
    {
        char m_signature[8];
        uint8_t m_checksum;
        char m_oemid[6];
        uint8_t m_revision;
        uint32_t m_rsdtAddress; // deprecated
        uint32_t m_length;
        uint64_t m_xsdtAddress;
        uint8_t m_extendedChecksum;
        uint8_t m_reserved[3];

        bool initialize();
    } __attribute__((packed));
} // namespace KtCore
