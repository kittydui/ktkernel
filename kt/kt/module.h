#pragma once
#include <cstddef>
#include <cstdint>

enum KtStatus : uint32_t
{
    FAILED = 0xB0000000,
    SUCCESS = 0xB0000001,
};

struct KtModule;

struct KtDispatch
{
    void (*Shutdown)(KtModule*);
    KtStatus (*Read)(KtModule*, void* buffer, size_t size);
    KtStatus (*Write)(KtModule*, const void* buffer, size_t size);
    KtStatus (*Close)(KtModule*);
};

struct KtModule
{
    KtDispatch m_dispatchFunctions;
    void* m_privateData;
};

using KtModuleEntry = KtStatus (*)(KtModule*);

// clang-format off
#define KtDeclareModule(name, entryPoint) \
    extern "C" __attribute__((used, section(".ktdrv"))) KtModuleEntry KtDriverEntry = entryPoint; \
    extern "C" __attribute__((used, section(".ktdrv"))) const char* KtDriverName = name;

// clang-format on
