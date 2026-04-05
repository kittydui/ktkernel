#include "mem/kernelstring.h"
#include "mem/memory.h"
#include "mem/string.h"

namespace kt_kernel
{

    kernel_string::kernel_string(const char* str)
    {
        if (!str)
            return;
        size_t len = strlen(str);
        ensure_capacity(len + 1);
        memcpy(chars, str, len);
        chars[len] = '\0';
        text_len = len;
    }

    kernel_string::~kernel_string()
    {
        if (chars)
            kfree(chars);
    }

    kernel_string::kernel_string(kernel_string&& other) noexcept
        : chars(other.chars), text_len(other.text_len), text_cap(other.text_cap)
    {
        other.chars = nullptr;
        other.text_len = 0;
        other.text_cap = 0;
    }

    kernel_string& kernel_string::operator=(kernel_string&& other) noexcept
    {
        if (this != &other) {
            if (chars)
                kfree(chars);

            chars = other.chars;
            text_len = other.text_len;
            text_cap = other.text_cap;
            other.chars = nullptr;
            other.text_len = 0;
            other.text_cap = 0;
        }
        return *this;
    }

    void kernel_string::ensure_capacity(size_t needed)
    {
        if (needed <= text_cap)
            return;

        size_t new_cap = text_cap == 0 ? 32 : text_cap;
        while (new_cap < needed)
            new_cap *= 2;

        auto* new_data = reinterpret_cast<char*>(kmalloc(new_cap));
        if (!new_data)
            return;

        if (chars) {
            memcpy(new_data, chars, text_len + 1);
            kfree(chars);
        }
        chars = new_data;
        text_cap = new_cap;
    }

    void kernel_string::append(char c)
    {
        ensure_capacity(text_len + 2);
        chars[text_len++] = c;
        chars[text_len] = '\0';
    }

    void kernel_string::append(const char* str)
    {
        if (!str)
            return;
        size_t len = strlen(str);
        ensure_capacity(text_len + len + 1);
        memcpy(chars + text_len, str, len);
        text_len += len;
        chars[text_len] = '\0';
    }

    void kernel_string::clear()
    {
        text_len = 0;
        if (chars)
            chars[0] = '\0';
    }

} // namespace kt_kernel
