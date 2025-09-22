#include <m2/math/Hash.h>

int32_t m2::HashI(const int32_t value) {
    uint8_t data[4] = {
        static_cast<uint8_t>(value),
        static_cast<uint8_t>(value >> 8),
        static_cast<uint8_t>(value >> 16),
        static_cast<uint8_t>(value >> 24),
    };
    return HashI(data, sizeof(data));
}
int32_t m2::HashI(const std::string& s) {
    return HashI(reinterpret_cast<const uint8_t*>(s.data()), s.length());
}
int32_t m2::HashI(const uint8_t* data, const size_t len) {
    int32_t h = 0x1ED980DD; // Random

    for (size_t i = 0; i < len; ++i) {
        auto tmp = static_cast<int32_t>(data[i]);
        tmp = tmp << (8 * (i % 4));
        h = h ^ tmp;
    }

    return h;
}
