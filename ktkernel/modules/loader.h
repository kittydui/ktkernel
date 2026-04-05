#pragma once
#include <kt/module.h>

namespace kt_kernel
{
    bool load_module(const char* module_path);
    kt_module* get_module(const char* module_name);

    bool load_core_modules();
} // namespace kt_kernel
