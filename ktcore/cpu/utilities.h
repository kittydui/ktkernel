#pragma once
#include "limine/requests.h"
#include <cstdint>

namespace KtCore
{
    uint64_t GetProcessorCount();
    void RunFunctionOnProcessor(uint32_t cpu, limine_goto_address func, void* arg);
} // namespace KtCore
