#pragma once
#include <cstdint>

[[noreturn]] inline void halt()
{
    while (true)
        asm volatile("hlt");
}

inline void invlpg(uint64_t addr)
{
    asm volatile("invlpg (%0)" ::"r"(addr) : "memory");
}

inline uint64_t read_cr3()
{
    uint64_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

inline void write_cr3(uint64_t cr3)
{
    asm volatile("mov %0, %%cr3" ::"r"(cr3) : "memory");
}

inline void outb(uint16_t port, uint8_t val)
{
    asm volatile("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

inline bool are_interrupts_enabled()
{
    unsigned long flags;
    asm volatile("pushf; pop %0" : "=rm"(flags) : : "memory");
    return (flags & (1 << 9)) != 0;
}

inline void io_wait()
{
    outb(0x80, 0);
}

struct __attribute__((packed)) gdt_ptr
{
    uint16_t limit;
    uint64_t base;
};

inline void load_gdt(gdt_ptr gdtr)
{
    asm volatile("lgdt %0" ::"m"(gdtr));
}

struct __attribute__((packed)) idt_entry
{
    uint16_t isr_low;
    uint16_t kernel_cs;
    uint8_t ist;
    uint8_t attributes;
    uint16_t isr_mid;
    uint32_t isr_high;
    uint32_t reserved;
};

struct __attribute__((packed)) idt_ptr
{
    uint16_t limit;
    uint64_t base;
};

inline void load_idt(idt_ptr idtr)
{
    asm volatile("lidt %0" ::"m"(idtr));
    asm volatile("sti");
}
