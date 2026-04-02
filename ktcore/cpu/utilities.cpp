#include "cpu/utilities.h"

namespace KtCore
{
    uint64_t GetProcessorCount()
    {
        return Limine::mpRequest.response->cpu_count;
    }

    void RunFunctionOnProcessor(uint32_t cpu, limine_goto_address func, void* arg)
    {
        auto* processor = Limine::mpRequest.response->cpus[cpu];
        processor->extra_argument = (uint64_t)arg;
        __atomic_store_n(&processor->goto_address, func, __ATOMIC_SEQ_CST);
    }
} // namespace KtCore
