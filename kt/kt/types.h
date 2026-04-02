#pragma once
#include <cstdint>

struct KtDateTime
{
    uint8_t m_Second;
    uint8_t m_Minute;
    uint8_t m_Hour;
    uint8_t m_Day;
    uint8_t m_Month;
    uint8_t m_Year;
    uint8_t m_Century;
};

class KtSpinLock
{
public:
    void acquire()
    {
        if (!__atomic_test_and_set(&m_locked, __ATOMIC_ACQUIRE))
            return;

        asm volatile("pause");
    }

    void release()
    {
        __atomic_clear(&m_locked, __ATOMIC_RELEASE);
    }

    bool tryAcquire()
    {
        return !__atomic_test_and_set(&m_locked, __ATOMIC_ACQUIRE);
    }

private:
    volatile bool m_locked = false;
};
