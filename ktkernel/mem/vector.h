#pragma once
#include "mem/alloc.h"
#include "mem/memory.h"
#include <cstddef>
#include <cstdint>

namespace kt_kernel
{

    template <typename T> class vector
    {
    public:
        vector() = default;

        ~vector()
        {
            if (buffer)
                kfree(buffer);
        }

        vector(const vector&) = delete;
        vector& operator=(const vector&) = delete;

        vector(vector&& other) noexcept : buffer(other.buffer), len(other.len), cap(other.cap)
        {
            other.buffer = nullptr;
            other.len = 0;
            other.cap = 0;
        }

        vector& operator=(vector&& other) noexcept
        {
            if (this != &other) {
                if (buffer)
                    kfree(buffer);
                buffer = other.buffer;
                len = other.len;
                cap = other.cap;
                other.buffer = nullptr;
                other.len = 0;
                other.cap = 0;
            }
            return *this;
        }

        void push_back(const T& value)
        {
            if (len >= cap)
                grow();
            buffer[len++] = value;
        }

        void pop_back()
        {
            if (len > 0)
                len--;
        }

        void clear()
        {
            len = 0;
        }

        T& operator[](size_t idx)
        {
            return buffer[idx];
        }

        const T& operator[](size_t idx) const
        {
            return buffer[idx];
        }

        T* data()
        {
            return buffer;
        }

        const T* data() const
        {
            return buffer;
        }

        [[nodiscard]] size_t size() const
        {
            return len;
        }

        [[nodiscard]] size_t capacity() const
        {
            return cap;
        }

        [[nodiscard]] bool empty() const
        {
            return len == 0;
        }

        T& back()
        {
            return buffer[len - 1];
        }

        const T& back() const
        {
            return buffer[len - 1];
        }

        void reserve(size_t new_cap)
        {
            if (new_cap <= cap)
                return;

            auto* new_data = reinterpret_cast<T*>(kmalloc(new_cap * sizeof(T)));
            if (!new_data)
                return;
            if (buffer) {
                memcpy(new_data, buffer, len * sizeof(T));
                kfree(buffer);
            }
            buffer = new_data;
            cap = new_cap;
        }

        void resize(size_t new_size)
        {
            if (new_size > cap)
                reserve(new_size);
            len = new_size;
        }

        void assign(const T* ptr, size_t count)
        {
            if (count > cap)
                reserve(count);

            if (ptr && count > 0)
                memcpy(buffer, ptr, count * sizeof(T));

            len = count;
        }

    private:
        void grow()
        {
            reserve(cap == 0 ? 16 : cap * 2);
        }

        T* buffer = nullptr;
        size_t len = 0;
        size_t cap = 0;
    };

} // namespace kt_kernel
