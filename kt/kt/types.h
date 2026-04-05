#pragma once
#include <cstdint>

struct kt_date_time
{
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t century;
};

class kt_spin_lock
{
public:
    void acquire()
    {
        if (!__atomic_test_and_set(&locked, __ATOMIC_ACQUIRE))
            return;

        asm volatile("pause");
    }

    void release()
    {
        __atomic_clear(&locked, __ATOMIC_RELEASE);
    }

    bool try_acquire()
    {
        return !__atomic_test_and_set(&locked, __ATOMIC_ACQUIRE);
    }

private:
    volatile bool locked = false;
};
