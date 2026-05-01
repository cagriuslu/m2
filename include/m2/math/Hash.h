#pragma once
#include <string>
#include <cstdint>

namespace m2 {
    /// For these functions, the initial values are intentionally designated as explicit becuase forgetting it could
    /// have drastic consequences.

    int32_t HashI(int32_t, int32_t initialValue);
    int32_t HashI(uint32_t, int32_t initialValue);
    int32_t HashI(int64_t, int32_t initialValue);
    int32_t HashI(uint64_t, int32_t initialValue);
    int32_t HashI(const std::string&, int32_t initialValue);
    int32_t HashI(const uint8_t*, size_t, int32_t initialValue);
}
