#include "cpu/utilities.h"

extern "C"
{
    using constructor_t = void (*)();
    constructor_t __init_array_start;
    constructor_t __init_array_end;
    void* __dso_handle = nullptr;

    void initialize_ctors()
    {
        for (constructor_t* i = &__init_array_start; i < &__init_array_end; i++)
            (*i)();
    }

    int __cxa_atexit(void (*)(void*), void*, void*)
    {
        return 0;
    }

    int __cxa_guard_acquire(long long* g)
    {
        return !(*(char*)g);
    }

    void __cxa_guard_release(long long* g)
    {
        *(char*)g = 1;
    }

    void __cxa_guard_abort(long long* g)
    {
        halt();
    }

    void __cxa_pure_virtual()
    {
        halt();
    }
}
