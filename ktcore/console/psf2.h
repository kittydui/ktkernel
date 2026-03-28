#pragma once
#include <cstdint>

namespace KtCore {
struct PSF2Header {
    uint32_t magic;
    uint32_t version;
    uint32_t headerSize;
    uint32_t flags;
    uint32_t length;
    uint32_t charSize;
    uint32_t height;
    uint32_t width;
};
} // namespace KtCore
