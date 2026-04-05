#pragma once
#include "mem/string.h"
#include "mem/vector.h"

namespace kt_kernel
{
    struct tar_header
    {
        char name[100];
        char mode[8];
        char uid[8];
        char gid[8];
        char size[12];
        char mtime[12];
        char checksum[8];
        char typeflag;
        char linkname[100];
        char magic[6];
        char version[2];
        char uname[32];
        char gname[32];
        char devmajor[8];
        char devminor[8];
        char prefix[155];
        char padding[12];
    };

    static_assert(sizeof(tar_header) == 512);

    class tar_archive
    {
    public:
        void open(const void* data, size_t size)
        {
            this->archive_data = reinterpret_cast<const uint8_t*>(data);
            this->archive_size = size;
        }

        struct tar_file
        {
            const uint8_t* data = nullptr;
            size_t size = 0;
            operator bool() const
            {
                return data != nullptr;
            }
        };

        tar_file read_file(const char* file_name)
        {
            size_t offset = 0;
            while (offset + 512 <= archive_size) {
                auto* header = reinterpret_cast<const tar_header*>(archive_data + offset);
                if (header->name[0] == '\0')
                    break;

                size_t file_size = parse_octal(header->size, 12);
                const uint8_t* data = archive_data + offset + 512;
                if (name_matches(header, file_name))
                    return { data, file_size };

                offset += 512 + align_up(file_size, 512);
            }

            return {};
        }

    private:
        static size_t parse_octal(const char* str, size_t len)
        {
            size_t result = 0;
            for (size_t i = 0; i < len && str[i] >= '0' && str[i] <= '7'; i++)
                result = (result * 8) + (str[i] - '0');
            return result;
        }

        static size_t align_up(size_t value, size_t alignment)
        {
            return (value + alignment - 1) & ~(alignment - 1);
        }

        static bool name_matches(const tar_header* hdr, const char* name)
        {
            const char* entry_name = hdr->name;
            if (entry_name[0] == '.' && entry_name[1] == '/')
                entry_name += 2;
            if (name[0] == '.' && name[1] == '/')
                name += 2;
            const char* a = entry_name;
            const char* b = name;
            return strcmp(a, b) == 0;
        }

        const uint8_t* archive_data = nullptr;
        size_t archive_size = 0;
    };
} // namespace kt_kernel
