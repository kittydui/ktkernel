#include "mem/kernelstring.h"
#include "mem/memory.h"
#include "mem/string.h"

namespace KtCore
{

KernelString::KernelString(const char* str)
{
    if (!str)
        return;
    size_t len = strlen(str);
    ensureCapacity(len + 1);
    memcpy(m_data, str, len);
    m_data[len] = '\0';
    m_length = len;
}

KernelString::~KernelString()
{
    if (m_data)
        KFree(m_data);
}

KernelString::KernelString(KernelString&& other) noexcept
    : m_data(other.m_data), m_length(other.m_length), m_capacity(other.m_capacity)
{
    other.m_data = nullptr;
    other.m_length = 0;
    other.m_capacity = 0;
}

KernelString& KernelString::operator=(KernelString&& other) noexcept
{
    if (this != &other) {
        if (m_data)
            KFree(m_data);

        m_data = other.m_data;
        m_length = other.m_length;
        m_capacity = other.m_capacity;
        other.m_data = nullptr;
        other.m_length = 0;
        other.m_capacity = 0;
    }
    return *this;
}

void KernelString::ensureCapacity(size_t needed)
{
    if (needed <= m_capacity)
        return;

    size_t new_cap = m_capacity == 0 ? 32 : m_capacity;
    while (new_cap < needed)
        new_cap *= 2;

    auto* new_data = reinterpret_cast<char*>(KMalloc(new_cap));
    if (!new_data)
        return;

    if (m_data) {
        memcpy(new_data, m_data, m_length + 1);
        KFree(m_data);
    }
    m_data = new_data;
    m_capacity = new_cap;
}

void KernelString::append(char c)
{
    ensureCapacity(m_length + 2);
    m_data[m_length++] = c;
    m_data[m_length] = '\0';
}

void KernelString::append(const char* str)
{
    if (!str)
        return;
    size_t len = strlen(str);
    ensureCapacity(m_length + len + 1);
    memcpy(m_data + m_length, str, len);
    m_length += len;
    m_data[m_length] = '\0';
}

void KernelString::clear()
{
    m_length = 0;
    if (m_data)
        m_data[0] = '\0';
}

} // namespace KtCore
