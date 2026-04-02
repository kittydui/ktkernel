#pragma once
#include <kt/module.h>

namespace KtKernel
{
    bool LoadModule(const char* modulePath);
    KtModule* GetModule(const char* moduleName);

    bool LoadCoreModules();
} // namespace KtKernel
