#include "modules/loader.h"
#include "limine/requests.h"
#include "subsystems/console/logging.h"
#include "symbol.h"
#include "systems.h"
#include "utilities/tar.h"
#include <kt/elf.h>
#include <kt/intrin.h>

extern "C" const KernelSymbol __ktsymbol_start[];
extern "C" const KernelSymbol __ktsymbol_end[];

static uint64_t ResolveKernelSymbol(const char* name)
{
    for (const KernelSymbol* sym = __ktsymbol_start; sym < __ktsymbol_end; sym++)
        if (!strcmp(sym->m_name, name))
            return reinterpret_cast<uint64_t>(sym->m_address);

    return 0;
}

struct KtDrv
{
    KtModuleEntry m_entryPoint;
    const char* m_moduleName;
};

struct LoadedModule
{
    const char* m_moduleName;
    KtModule m_module;
    bool m_active;
    KtDrv* m_ktdrv;
};

static constexpr uint64_t PAGE_SIZE = 0x1000;

extern "C" char kernel_end[];

static uint64_t s_moduleAllocNext = 0;
static uint64_t s_moduleAllocPageEnd = 0;

static void* ModuleAlloc(size_t size)
{
    if (size == 0)
        return nullptr;

    if (s_moduleAllocNext == 0) {
        s_moduleAllocNext = (reinterpret_cast<uint64_t>(kernel_end) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        s_moduleAllocPageEnd = s_moduleAllocNext;
    }

    s_moduleAllocNext = (s_moduleAllocNext + 15) & ~15ULL;

    uint64_t result = s_moduleAllocNext;
    s_moduleAllocNext += size;

    while (s_moduleAllocPageEnd < s_moduleAllocNext) {
        uint64_t frame = KtCore::g_kernelContext->m_pmm->allocateFrame();
        if (!frame)
            return nullptr;
        if (!KtCore::g_kernelContext->m_vmm->map(
                s_moduleAllocPageEnd, frame, KtCore::PageFlags::PRESENT | KtCore::PageFlags::WRITABLE))
            return nullptr;
        s_moduleAllocPageEnd += PAGE_SIZE;
    }

    return reinterpret_cast<void*>(result);
}

namespace KtKernel
{
    static KtCore::Vector<LoadedModule*> loadedModules = {};

    bool VerifyELF(const Elf64_Ehdr* e_hdr)
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

    bool AllocateSections(const uint8_t* base, const Elf64_Ehdr* e_hdr, uint64_t** section_addrs, uint16_t* section_num)
    {
        const auto* section_headers = reinterpret_cast<const Elf64_Shdr*>(base + e_hdr->e_shoff);
        *section_num = e_hdr->e_shnum;

        *section_addrs = static_cast<uint64_t*>(KtCore::KMalloc(*section_num * sizeof(uint64_t)));
        memset(*section_addrs, 0, *section_num * sizeof(uint64_t));

        for (uint16_t i = 0; i < *section_num; i++) {
            const Elf64_Shdr& shdr = section_headers[i];
            if (!(shdr.sh_flags & SHF_ALLOC))
                continue;

            if (shdr.sh_size == 0)
                continue;

            void* mem = ModuleAlloc(shdr.sh_size);
            if (!mem) {
                KtCore::KPrint("Couldn't allocate module!");
                KtCore::KFree(*section_addrs);
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

    bool LoadModule(const char* modulePath)
    {
        auto* system_tar = Limine::moduleRequest.response->modules[0];
        TarArchive systems_archive;
        systems_archive.open(system_tar->address, system_tar->size);

        auto file = systems_archive.readFile(modulePath);
        if (!file) {
            KtCore::KPrint("Cannot find file to load.");
            return false;
        }

        const uint8_t* base = file.m_data;
        const auto* e_hdr = reinterpret_cast<const Elf64_Ehdr*>(base);

        if (!VerifyELF(e_hdr)) {
            KtCore::KPrint("Unable to verify elf header.");
            return false;
        }

        uint64_t* section_addrs = nullptr;
        uint16_t section_num = 0;
        if (!AllocateSections(base, e_hdr, &section_addrs, &section_num)) {
            KtCore::KPrint("Unable to allocate sections.");
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
            KtCore::KFree(section_addrs);
            return false;
        }

        const auto* syms = reinterpret_cast<const Elf64_Sym*>(base + symtab->sh_offset);
        size_t sym_count = symtab->sh_size / sizeof(Elf64_Sym);
        auto* sym_addrs = static_cast<uint64_t*>(KtCore::KMalloc(sym_count * sizeof(uint64_t)));
        memset(sym_addrs, 0, sym_count * sizeof(uint64_t));

        for (size_t i = 1; i < sym_count; i++) {
            const Elf64_Sym& sym = syms[i];

            if (sym.st_shndx == SHN_UNDEF) {
                const char* name = strtab + sym.st_name;
                sym_addrs[i] = ResolveKernelSymbol(name);
                if (!sym_addrs[i]) {
                    KtCore::KPrint("Unresolved symbol '{}'.", name);
                    KtCore::KFree(sym_addrs);
                    KtCore::KFree(section_addrs);
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
                        KtCore::KFree(sym_addrs);
                        KtCore::KFree(section_addrs);
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
                    KtCore::KPrint("Unknown reloc type {}.", type);
                    KtCore::KFree(sym_addrs);
                    KtCore::KFree(section_addrs);
                    return false;
                }
            }
        }

        KtCore::KFree(sym_addrs);
        const char* shstrtab = reinterpret_cast<const char*>(base + section_headers[e_hdr->e_shstrndx].sh_offset);

        KtDrv* ktdrv_section = nullptr;

        for (uint16_t i = 0; i < section_num; i++) {
            if (!strcmp(shstrtab + section_headers[i].sh_name, ".ktdrv") && section_addrs[i] != 0) {
                ktdrv_section = reinterpret_cast<KtDrv*>(section_addrs[i]);
                break;
            }
        }

        if (ktdrv_section == nullptr) {
            KtCore::KPrint(".ktdrv section not found.");
            KtCore::KFree(section_addrs);
            return false;
        }

        auto* loaded_module = static_cast<LoadedModule*>(KtCore::KMalloc(sizeof(LoadedModule)));
        loaded_module->m_moduleName = ktdrv_section->m_moduleName;
        loaded_module->m_ktdrv = ktdrv_section;

        KtStatus status = ktdrv_section->m_entryPoint(&loaded_module->m_module);
        if (status != KtStatus::SUCCESS) {
            KtCore::KPrint("Module entrypoint exited with status '{}'.", status);
            KtCore::KFree(loaded_module);
            KtCore::KFree(section_addrs);
            return false;
        }

        loaded_module->m_active = true;
        loadedModules.pushBack(loaded_module);
        KtCore::KFree(section_addrs);
        return true;
    }

    KtModule* GetModule(const char* moduleName)
    {
        for (int i = 0; i < loadedModules.size(); i++) {
            auto& mod = loadedModules[i];
            if (!strcmp(mod->m_moduleName, moduleName))
                return &mod->m_module;
        }

        return nullptr;
    }
} // namespace KtKernel
