#pragma once
#include "mem/alloc.h"
#include <cstddef>
#include <cstdint>

namespace KtCore
{

    class KernelString
    {
    public:
        KernelString() = default;
        explicit KernelString(const char* str);
        ~KernelString();

        KernelString(const KernelString&) = delete;
        KernelString& operator=(const KernelString&) = delete;

        KernelString(KernelString&& other) noexcept;
        KernelString& operator=(KernelString&& other) noexcept;

        [[nodiscard]] const char* cStr() const
        {
            return m_data ? m_data : "";
        }

        [[nodiscard]] size_t length() const
        {
            return m_length;
        }

        [[nodiscard]] bool empty() const
        {
            return m_length == 0;
        }

        void append(char c);
        void append(const char* str);
        void clear();

        char operator[](size_t idx) const
        {
            return m_data[idx];
        }

        char* data()
        {
            return m_data;
        }

    private:
        char* m_data = nullptr;
        size_t m_length = 0;
        size_t m_capacity = 0;

        void ensureCapacity(size_t needed);
    };

    inline KernelString AllocateKernelString(const char* str)
    {
        return KernelString(str);
    }

} // namespace KtCore
