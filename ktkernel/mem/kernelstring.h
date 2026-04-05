#pragma once
#include "mem/alloc.h"
#include <cstddef>
#include <cstdint>

namespace kt_kernel
{

    class kernel_string
    {
    public:
        kernel_string() = default;
        explicit kernel_string(const char* str);
        ~kernel_string();

        kernel_string(const kernel_string&) = delete;
        kernel_string& operator=(const kernel_string&) = delete;

        kernel_string(kernel_string&& other) noexcept;
        kernel_string& operator=(kernel_string&& other) noexcept;

        [[nodiscard]] const char* c_str() const
        {
            return chars ? chars : "";
        }

        [[nodiscard]] size_t length() const
        {
            return text_len;
        }

        [[nodiscard]] bool empty() const
        {
            return text_len == 0;
        }

        void append(char c);
        void append(const char* str);
        void clear();

        char operator[](size_t idx) const
        {
            return chars[idx];
        }

        char* data()
        {
            return chars;
        }

    private:
        char* chars = nullptr;
        size_t text_len = 0;
        size_t text_cap = 0;

        void ensure_capacity(size_t needed);
    };

    inline kernel_string allocate_kernel_string(const char* str)
    {
        return kernel_string(str);
    }

} // namespace kt_kernel
