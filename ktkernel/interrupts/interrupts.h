#pragma once
#include <kt/intrin.h>

namespace kt_kernel
{
    constexpr auto idt_max_descriptors = 256;
    __attribute__((aligned(0x10))) inline idt_entry idt[idt_max_descriptors] = {};

    using interrupt_handler_fn = void (*)(uint8_t interrupt_index, uint64_t error_code);
    inline interrupt_handler_fn interrupt_handlers[idt_max_descriptors] = {};

    enum class gate_type : uint8_t
    {
        interrupt_gate = 0xE,
        trap_gate = 0xF
    };

    enum class dpl_ring : uint8_t
    {
        ring0 = 0,
        ring1 = 1,
        ring2 = 2,
        ring3 = 3,
    };

    uint8_t create_attributes(uint8_t dpl, gate_type gate, bool p = true);

    bool setup_idt();
    void idt_set_gate_descriptor(uint8_t index, void* isr, uint8_t flags);

    void register_interrupt_handler(uint8_t interrupt_number, uint8_t attributes, interrupt_handler_fn handler);
} // namespace kt_kernel
