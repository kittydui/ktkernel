#pragma once
#include <kt/intrin.h>

namespace KtKernel
{
    constexpr auto IDT_MAX_DESCRIPTORS = 256;
    __attribute__((aligned(0x10))) inline IDTEntry IDT[IDT_MAX_DESCRIPTORS] = {};

    using InterruptHandler_t = void (*)(uint8_t interruptIndex, uint64_t errorCode);
    inline InterruptHandler_t interruptHandlers[IDT_MAX_DESCRIPTORS] = {};

    enum GateType : uint8_t
    {
        InterruptGate = 0xE,
        TrapGate = 0xF
    };

    enum DPLRing : uint8_t
    {
        Ring0 = 0,
        Ring1 = 1,
        Ring2 = 2,
        Ring3 = 3,
    };

    uint8_t CreateAttributes(uint8_t dpl, GateType gateType, bool p = true);

    bool SetupIdt();
    void IdtSetGateDescriptor(uint8_t index, void* isr, uint8_t flags);

    void RegisterInterruptHandler(uint8_t interruptNumber, uint8_t attributes, InterruptHandler_t handler);

    void Int3Handler(uint8_t interruptIndex, uint64_t errorCode);
    void GPHandler(uint8_t interruptIndex, uint64_t errorCode);

    [[noreturn]] void ExceptionHandler();
} // namespace KtKernel
