#pragma once
#include <cstddef>
#include <cstdint>

enum class kt_status : uint32_t
{
    failed = 0xB0000000,
    success = 0xB0000001,
};

struct kt_module;

struct kt_dispatch
{
    void (*shutdown)(kt_module*);
    kt_status (*read)(kt_module*, void* buffer, size_t size);
    kt_status (*write)(kt_module*, const void* buffer, size_t size);
    kt_status (*close)(kt_module*);
};

struct kt_module
{
    kt_dispatch dispatch_functions;
    void* private_data;
};

using kt_module_entry = kt_status (*)(kt_module*);

// clang-format off
#define kt_declare_module(name, entry_point) \
    extern "C" __attribute__((used, section(".ktdrv"))) kt_module_entry kt_driver_entry = entry_point; \
    extern "C" __attribute__((used, section(".ktdrv"))) const char* kt_driver_name = name;

// clang-format on
