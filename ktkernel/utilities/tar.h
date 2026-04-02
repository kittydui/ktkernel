#pragma once
#include "mem/string.h"
#include "mem/vector.h"

namespace KtKernel
{
    struct TarHeader
    {
        char m_name[100];
        char m_mode[8];
        char m_uid[8];
        char m_gid[8];
        char m_size[12];
        char m_time[12];
        char m_checksum[8];
        char m_typeflag;
        char m_linkname[100];
        char m_magic[6];
        char m_version[2];
        char m_uname[32];
        char m_gname[32];
        char m_devmajor[8];
        char m_devminor[8];
        char m_prefix[155];
        char m_padding[12];
    };

    static_assert(sizeof(TarHeader) == 512);

    class TarArchive
    {
    public:
        void open(const void* data, size_t size)
        {
            this->m_data = reinterpret_cast<const uint8_t*>(data);
            this->m_size = size;
        }

        struct TarFile
        {
            const uint8_t* m_data = nullptr;
            size_t m_size = 0;
            operator bool() const
            {
                return m_data != nullptr;
            }
        };

        TarFile readFile(const char* fileName)
        {
            size_t offset = 0;
            while (offset + 512 <= m_size) {
                auto* header = reinterpret_cast<const TarHeader*>(m_data + offset);
                if (header->m_name[0] == '\0')
                    break;

                size_t file_size = parseOctal(header->m_size, 12);
                const uint8_t* data = m_data + offset + 512;
                if (nameMatches(header, fileName))
                    return { data, file_size };

                offset += 512 + alignUp(file_size, 512);
            }

            return {};
        }

    private:
        static size_t parseOctal(const char* str, size_t len)
        {
            size_t result = 0;
            for (size_t i = 0; i < len && str[i] >= '0' && str[i] <= '7'; i++)
                result = (result * 8) + (str[i] - '0');
            return result;
        }

        static size_t alignUp(size_t value, size_t alignment)
        {
            return (value + alignment - 1) & ~(alignment - 1);
        }

        static bool nameMatches(const TarHeader* hdr, const char* name)
        {
            const char* entry_name = hdr->m_name;
            if (entry_name[0] == '.' && entry_name[1] == '/')
                entry_name += 2;
            if (name[0] == '.' && name[1] == '/')
                name += 2;
            const char* a = entry_name;
            const char* b = name;
            return strcmp(a, b) == 0;
        }

        const uint8_t* m_data;
        size_t m_size;
    };
} // namespace KtKernel
