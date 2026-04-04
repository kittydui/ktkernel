#include "interrupts/interrupts.h"
#include "utilities/gdt.h"

extern void* isrStubTable[];

namespace KtKernel
{
    bool SetupIdt()
    {
        IDTR idtr = {};
        idtr.m_base = (uintptr_t)&IDT[0];
        idtr.m_limit = (uint16_t)sizeof(IDTEntry) * IDT_MAX_DESCRIPTORS - 1;

        RegisterInterruptHandler(3, CreateAttributes(DPLRing::Ring0, GateType::TrapGate), Int3Handler);
        RegisterInterruptHandler(13, CreateAttributes(DPLRing::Ring0, GateType::TrapGate), GPHandler);

        loadIdt(idtr);
        return true;
    }

    void IdtSetGateDescriptor(uint8_t index, void* isr, uint8_t flags)
    {
        IDTEntry* descriptor = &IDT[index];
        descriptor->m_isrLow = (uint64_t)isr & 0xFFFF;
        descriptor->m_kernelCs = KERNEL_CODE_SELECTOR;
        descriptor->m_ist = 0;
        descriptor->m_attributes = flags;
        descriptor->m_isrMid = ((uint64_t)isr >> 16) & 0xFFFF;
        descriptor->m_isrHigh = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
        descriptor->m_reserved = 0;
    }

    extern "C" void InterruptDispatch(uint8_t interruptNumber, uint64_t errorCode)
    {
        if (interruptHandlers[interruptNumber])
            interruptHandlers[interruptNumber](interruptNumber, errorCode);
        else
            ExceptionHandler();
    }

    uint8_t CreateAttributes(uint8_t dpl, GateType gateType, bool p)
    {
        return ((uint8_t)p << 7) | ((dpl & 0x3) << 5) | (gateType & 0xF);
    }

    void RegisterInterruptHandler(uint8_t interruptNumber, uint8_t attributes, InterruptHandler_t handler)
    {
        IdtSetGateDescriptor(interruptNumber, isrStubTable[interruptNumber], attributes);

        if (!interruptHandlers[interruptNumber])
            interruptHandlers[interruptNumber] = handler;
    }
} // namespace KtKernel
