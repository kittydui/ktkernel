#include "interrupts/interrupts.h"
#include "interrupts/handlers.h"
#include "utilities/gdt.h"

extern void* isr_stub_table[];

namespace kt_kernel
{
    bool setup_idt()
    {
        idt_ptr idtr = {};
        idtr.base = (uintptr_t)&idt[0];
        idtr.limit = (uint16_t)sizeof(idt_entry) * idt_max_descriptors - 1;

        register_interrupt_handler(static_cast<uint8_t>(interrupt_vector::bp),
                                   create_attributes(static_cast<uint8_t>(dpl_ring::ring0), gate_type::trap_gate),
                                   int3_handler);
        register_interrupt_handler(static_cast<uint8_t>(interrupt_vector::gp),
                                   create_attributes(static_cast<uint8_t>(dpl_ring::ring0), gate_type::trap_gate),
                                   gp_handler);

        load_idt(idtr);
        return true;
    }

    void idt_set_gate_descriptor(uint8_t index, void* isr, uint8_t flags)
    {
        idt_entry* descriptor = &idt[index];
        descriptor->isr_low = (uint64_t)isr & 0xFFFF;
        descriptor->kernel_cs = kernel_code_selector;
        descriptor->ist = 0;
        descriptor->attributes = flags;
        descriptor->isr_mid = ((uint64_t)isr >> 16) & 0xFFFF;
        descriptor->isr_high = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
        descriptor->reserved = 0;
    }

    extern "C" void interrupt_dispatch(uint8_t interrupt_number, uint64_t error_code)
    {
        if (interrupt_handlers[interrupt_number])
            interrupt_handlers[interrupt_number](interrupt_number, error_code);
        else
            exception_handler();
    }

    uint8_t create_attributes(uint8_t dpl, gate_type gate, bool p)
    {
        return ((uint8_t)p << 7) | ((dpl & 0x3) << 5) | (static_cast<uint8_t>(gate) & 0xF);
    }

    void register_interrupt_handler(uint8_t interrupt_number, uint8_t attributes, interrupt_handler_fn handler)
    {
        idt_set_gate_descriptor(interrupt_number, isr_stub_table[interrupt_number], attributes);

        if (!interrupt_handlers[interrupt_number])
            interrupt_handlers[interrupt_number] = handler;
    }
} // namespace kt_kernel
