#pragma once

#include <cstdint>

struct Elf64_Ehdr
{
    uint8_t e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff; // section header table offset
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;    // number of section headers
    uint16_t e_shstrndx; // index of section name string table
};
struct Elf64_Shdr
{
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset; // offset in file to section data
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
};
struct Elf64_Sym
{
    uint32_t st_name;
    uint8_t st_info;
    uint8_t st_other;
    uint16_t st_shndx; // section index this symbol is defined in
    uint64_t st_value; // offset within that section
    uint64_t st_size;
};
struct Elf64_Rela
{
    uint64_t r_offset; // where in the section to patch
    uint64_t r_info;   // symbol index + relocation type
    int64_t r_addend;
};

#define ELF64_R_SYM(info) ((info) >> 32)
#define ELF64_R_TYPE(info) ((info) & 0xFFFFFFFF)
#define ELF64_ST_BIND(info) ((info) >> 4)
#define ELF64_ST_TYPE(info) ((info) & 0xF)
// Constants
constexpr uint16_t ET_REL = 1;
constexpr uint16_t EM_X86_64 = 62;
constexpr uint32_t SHT_PROGBITS = 1;
constexpr uint32_t SHT_SYMTAB = 2;
constexpr uint32_t SHT_STRTAB = 3;
constexpr uint32_t SHT_RELA = 4;
constexpr uint32_t SHT_NOBITS = 8;
constexpr uint64_t SHF_ALLOC = 0x2;
constexpr uint16_t SHN_UNDEF = 0;
constexpr uint16_t SHN_ABS = 0xFFF1;
constexpr uint32_t R_X86_64_64 = 1;    // S + A
constexpr uint32_t R_X86_64_PC32 = 2;  // S + A - P
constexpr uint32_t R_X86_64_32 = 10;   // S + A (truncated to 32-bit)
constexpr uint32_t R_X86_64_32S = 11;  // S + A (signed 32-bit)
constexpr uint32_t R_X86_64_PLT32 = 4; // S + A - P (same as PC32 for us)
