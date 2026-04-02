#pragma once
#include "mem/alloc.h"
#include "mem/memory.h"
#include <cstddef>
#include <cstdint>

namespace KtCore
{

    template <typename T> class Vector
    {
    public:
        Vector() = default;

        ~Vector()
        {
            if (m_data)
                KFree(m_data);
        }

        Vector(const Vector&) = delete;
        Vector& operator=(const Vector&) = delete;

        Vector(Vector&& other) noexcept : m_data(other.m_data), m_size(other.m_size), m_capacity(other.m_capacity)
        {
            other.m_data = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
        }

        Vector& operator=(Vector&& other) noexcept
        {
            if (this != &other) {
                if (m_data)
                    KFree(m_data);
                m_data = other.m_data;
                m_size = other.m_size;
                m_capacity = other.m_capacity;
                other.m_data = nullptr;
                other.m_size = 0;
                other.m_capacity = 0;
            }
            return *this;
        }

        void pushBack(const T& value)
        {
            if (m_size >= m_capacity)
                grow();
            m_data[m_size++] = value;
        }

        void popBack()
        {
            if (m_size > 0)
                m_size--;
        }

        void clear()
        {
            m_size = 0;
        }

        T& operator[](size_t idx)
        {
            return m_data[idx];
        }

        const T& operator[](size_t idx) const
        {
            return m_data[idx];
        }

        T* data()
        {
            return m_data;
        }

        const T* data() const
        {
            return m_data;
        }

        [[nodiscard]] size_t size() const
        {
            return m_size;
        }

        [[nodiscard]] size_t capacity() const
        {
            return m_capacity;
        }

        [[nodiscard]] bool empty() const
        {
            return m_size == 0;
        }

        T& back()
        {
            return m_data[m_size - 1];
        }

        const T& back() const
        {
            return m_data[m_size - 1];
        }

        void reserve(size_t newCap)
        {
            if (newCap <= m_capacity)
                return;

            auto* new_data = reinterpret_cast<T*>(KMalloc(newCap * sizeof(T)));
            if (!new_data)
                return;
            if (m_data) {
                memcpy(new_data, m_data, m_size * sizeof(T));
                KFree(m_data);
            }
            m_data = new_data;
            m_capacity = newCap;
        }

        void resize(size_t newSize)
        {
            if (newSize > m_capacity)
                reserve(newSize);
            m_size = newSize;
        }

        void assign(const T* ptr, size_t count)
        {
            if (count > m_capacity)
                reserve(count);

            if (ptr && count > 0)
                memcpy(m_data, ptr, count * sizeof(T));

            m_size = count;
        }

    private:
        void grow()
        {
            reserve(m_capacity == 0 ? 16 : m_capacity * 2);
        }

        T* m_data = nullptr;
        size_t m_size = 0;
        size_t m_capacity = 0;
    };

} // namespace KtCore
