#include <m2/math/Hash.h>

int32_t m2::HashI(const int32_t value, const int32_t initialValue) {
    const uint8_t data[4] = {
        static_cast<uint8_t>(value),
        static_cast<uint8_t>(value >> 8),
        static_cast<uint8_t>(value >> 16),
        static_cast<uint8_t>(value >> 24),
    };
    return HashI(data, sizeof(data), initialValue);
}
int32_t m2::HashI(const uint32_t value, const int32_t initialValue) {
    return HashI(static_cast<int32_t>(value), initialValue);
}
int32_t m2::HashI(const int64_t value, const int32_t initialValue) {
    const uint8_t data[8] = {
        static_cast<uint8_t>(value),
        static_cast<uint8_t>(value >> 8),
        static_cast<uint8_t>(value >> 16),
        static_cast<uint8_t>(value >> 24),
        static_cast<uint8_t>(value >> 32),
        static_cast<uint8_t>(value >> 40),
        static_cast<uint8_t>(value >> 48),
        static_cast<uint8_t>(value >> 56),
    };
    return HashI(data, sizeof(data), initialValue);
}
int32_t m2::HashI(const uint64_t value, const int32_t initialValue) {
    return HashI(static_cast<int64_t>(value), initialValue);
}
int32_t m2::HashI(const std::string& s, const int32_t initialValue) {
    return HashI(reinterpret_cast<const uint8_t*>(s.data()), s.length(), initialValue);
}
int32_t m2::HashI(const uint8_t* data, const size_t len, const int32_t initialValue) {
    int32_t h = initialValue ? initialValue : 0x1ED980DD; // Random

    for (size_t i = 0; i < len; ++i) {
        auto tmp = static_cast<int32_t>(data[i]);
        tmp = tmp << (8 * (i % 4));
        h = h ^ tmp;
    }

    return h;
}
