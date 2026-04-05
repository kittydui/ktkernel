#include "modules/loader.h"
#include "limine/requests.h"
#include "mem/string.h"
#include "subsystems/console/logging.h"
#include "subsystems/vmm/vmm.h"
#include "symbol.h"
#include "systems.h"
#include "utilities/tar.h"
#include <kt/elf.h>
#include <kt/intrin.h>

extern "C" const kernel_symbol __ktsymbol_start[];
extern "C" const kernel_symbol __ktsymbol_end[];

static uint64_t resolve_kernel_symbol(const char* name)
{
    for (const kernel_symbol* sym = __ktsymbol_start; sym < __ktsymbol_end; sym++)
        if (!strcmp(sym->name, name))
            return reinterpret_cast<uint64_t>(sym->address);

    return 0;
}

struct kt_drv
{
    kt_module_entry entry_point;
    const char* module_name;
};

struct loaded_module
{
    const char* module_name;
    kt_module mod;
    bool active;
    kt_drv* ktdrv;
};

static constexpr uint64_t page_sz = 0x1000;

extern "C" char kernel_end[];

static uint64_t s_module_alloc_next = 0;
static uint64_t s_module_alloc_page_end = 0;

static void* module_alloc(size_t size)
{
    if (size == 0)
        return nullptr;

    if (s_module_alloc_next == 0) {
        s_module_alloc_next = (reinterpret_cast<uint64_t>(kernel_end) + page_sz - 1) & ~(page_sz - 1);
        s_module_alloc_page_end = s_module_alloc_next;
    }

    s_module_alloc_next = (s_module_alloc_next + 15) & ~15ULL;

    uint64_t result = s_module_alloc_next;
    s_module_alloc_next += size;

    while (s_module_alloc_page_end < s_module_alloc_next) {
        uint64_t frame = kt_kernel::g_kernel_context->pmm->allocate_frame();
        if (!frame)
            return nullptr;
        if (!kt_kernel::g_kernel_context->vmm->map(
                s_module_alloc_page_end,
                frame,
                static_cast<uint64_t>(kt_kernel::page_flags::present | kt_kernel::page_flags::writable)))
            return nullptr;
        s_module_alloc_page_end += page_sz;
    }

    return reinterpret_cast<void*>(result);
}

namespace kt_kernel
{
    static vector<loaded_module*> loaded_modules = {};

    static bool verify_elf(const Elf64_Ehdr* e_hdr)
    {
        if (e_hdr->e_ident[0] != 0x7F || e_hdr->e_ident[1] != 'E' || e_hdr->e_ident[2] != 'L' ||
            e_hdr->e_ident[3] != 'F')
            return false;

        if (e_hdr->e_type != ET_REL)
            return false;

        if (e_hdr->e_machine != EM_X86_64)
            return false;

        return true;
    }

    static bool
    allocate_sections(const uint8_t* base, const Elf64_Ehdr* e_hdr, uint64_t** section_addrs, uint16_t* section_num)
    {
        const auto* section_headers = reinterpret_cast<const Elf64_Shdr*>(base + e_hdr->e_shoff);
        *section_num = e_hdr->e_shnum;

        *section_addrs = static_cast<uint64_t*>(kmalloc(*section_num * sizeof(uint64_t)));
        memset(*section_addrs, 0, *section_num * sizeof(uint64_t));

        for (uint16_t i = 0; i < *section_num; i++) {
            const Elf64_Shdr& shdr = section_headers[i];
            if (!(shdr.sh_flags & SHF_ALLOC))
                continue;

            if (shdr.sh_size == 0)
                continue;

            void* mem = module_alloc(shdr.sh_size);
            if (!mem) {
                print("Couldn't allocate module!");
                kfree(*section_addrs);
                return false;
            }

            if (shdr.sh_type == SHT_NOBITS)
                memset(mem, 0, shdr.sh_size);
            else
                memcpy(mem, base + shdr.sh_offset, shdr.sh_size);

            (*section_addrs)[i] = reinterpret_cast<uint64_t>(mem);
        }

        return true;
    }

    bool load_module(const char* module_path)
    {
        auto* system_tar = limine::module_request.response->modules[0];
        tar_archive systems_archive;
        systems_archive.open(system_tar->address, system_tar->size);

        auto file = systems_archive.read_file(module_path);
        if (!file) {
            print("Cannot find file to load.");
            return false;
        }

        const uint8_t* base = file.data;
        const auto* e_hdr = reinterpret_cast<const Elf64_Ehdr*>(base);

        if (!verify_elf(e_hdr)) {
            print("Unable to verify elf header.");
            return false;
        }

        uint64_t* section_addrs = nullptr;
        uint16_t section_num = 0;
        if (!allocate_sections(base, e_hdr, &section_addrs, &section_num)) {
            print("Unable to allocate sections.");
            return false;
        }

        const auto* section_headers = reinterpret_cast<const Elf64_Shdr*>(base + e_hdr->e_shoff);
        const Elf64_Shdr* symtab = nullptr;
        const char* strtab = nullptr;

        for (uint16_t i = 0; i < section_num; i++) {
            if (section_headers[i].sh_type == SHT_SYMTAB) {
                symtab = &section_headers[i];
                strtab = reinterpret_cast<const char*>(base + section_headers[symtab->sh_link].sh_offset);
                break;
            }
        }

        if (!symtab) {
            kfree(section_addrs);
            return false;
        }

        const auto* syms = reinterpret_cast<const Elf64_Sym*>(base + symtab->sh_offset);
        size_t sym_count = symtab->sh_size / sizeof(Elf64_Sym);
        auto* sym_addrs = static_cast<uint64_t*>(kmalloc(sym_count * sizeof(uint64_t)));
        memset(sym_addrs, 0, sym_count * sizeof(uint64_t));

        for (size_t i = 1; i < sym_count; i++) {
            const Elf64_Sym& sym = syms[i];

            if (sym.st_shndx == SHN_UNDEF) {
                const char* name = strtab + sym.st_name;
                sym_addrs[i] = resolve_kernel_symbol(name);
                if (!sym_addrs[i]) {
                    print("Unresolved symbol '{}'.", name);
                    kfree(sym_addrs);
                    kfree(section_addrs);
                    return false;
                }
            } else if (sym.st_shndx == SHN_ABS) {
                sym_addrs[i] = sym.st_value;
            } else {
                sym_addrs[i] = section_addrs[sym.st_shndx] + sym.st_value;
            }
        }

        for (uint16_t i = 0; i < section_num; i++) {
            if (section_headers[i].sh_type != SHT_RELA)
                continue;

            uint16_t target = section_headers[i].sh_info;
            if (!section_addrs[target])
                continue;

            const auto* relas = reinterpret_cast<const Elf64_Rela*>(base + section_headers[i].sh_offset);
            size_t rela_count = section_headers[i].sh_size / sizeof(Elf64_Rela);
            for (size_t j = 0; j < rela_count; j++) {
                uint32_t sym_idx = ELF64_R_SYM(relas[j].r_info);
                uint32_t type = ELF64_R_TYPE(relas[j].r_info);
                uint64_t P = section_addrs[target] + relas[j].r_offset;
                uint64_t S = sym_addrs[sym_idx];
                int64_t A = relas[j].r_addend;

                switch (type) {
                case R_X86_64_64:
                    *reinterpret_cast<uint64_t*>(P) = S + A;
                    break;

                case R_X86_64_PC32:
                case R_X86_64_PLT32: {
                    int64_t value = static_cast<int64_t>(S + A) - static_cast<int64_t>(P);
                    if (value > 0x7FFFFFFFLL || value < -0x80000000LL) {
                        kfree(sym_addrs);
                        kfree(section_addrs);
                        return false;
                    }
                    *reinterpret_cast<int32_t*>(P) = static_cast<int32_t>(value);
                    break;
                }

                case R_X86_64_32:
                    *reinterpret_cast<uint32_t*>(P) = static_cast<uint32_t>(S + A);
                    break;
                case R_X86_64_32S:
                    *reinterpret_cast<int32_t*>(P) = static_cast<int32_t>(S + A);
                    break;
                default:
                    print("Unknown reloc type {}.", type);
                    kfree(sym_addrs);
                    kfree(section_addrs);
                    return false;
                }
            }
        }

        kfree(sym_addrs);
        const char* shstrtab = reinterpret_cast<const char*>(base + section_headers[e_hdr->e_shstrndx].sh_offset);

        kt_drv* ktdrv_section = nullptr;

        for (uint16_t i = 0; i < section_num; i++) {
            if (!strcmp(shstrtab + section_headers[i].sh_name, ".ktdrv") && section_addrs[i] != 0) {
                ktdrv_section = reinterpret_cast<kt_drv*>(section_addrs[i]);
                break;
            }
        }

        if (ktdrv_section == nullptr) {
            print(".ktdrv section not found.");
            kfree(section_addrs);
            return false;
        }

        auto* loaded = static_cast<loaded_module*>(kmalloc(sizeof(loaded_module)));
        loaded->module_name = ktdrv_section->module_name;
        loaded->ktdrv = ktdrv_section;

        kt_status status = ktdrv_section->entry_point(&loaded->mod);
        if (status != kt_status::success) {
            print("Module entrypoint exited with status '{}'.", static_cast<uint32_t>(status));
            kfree(loaded);
            kfree(section_addrs);
            return false;
        }

        loaded->active = true;
        loaded_modules.push_back(loaded);
        kfree(section_addrs);
        return true;
    }

    kt_module* get_module(const char* module_name)
    {
        for (size_t i = 0; i < loaded_modules.size(); i++) {
            auto* mod = loaded_modules[i];
            if (!strcmp(mod->module_name, module_name))
                return &mod->mod;
        }

        return nullptr;
    }
} // namespace kt_kernel
