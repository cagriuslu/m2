#pragma once
#include <string>
#include <cstdint>

namespace m2 {
    int32_t HashI(int32_t);
    int32_t HashI(const std::string&);
    int32_t HashI(const uint8_t*, size_t);
}
