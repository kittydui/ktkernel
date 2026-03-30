#pragma once
#include <cstddef>

namespace KtCore
{
void* KMalloc(size_t size);
void KFree(void* ptr);
} // namespace KtCore
