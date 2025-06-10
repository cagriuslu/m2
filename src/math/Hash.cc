#include <m2/math/Hash.h>

int32_t m2::IHash(const std::string& s) {
    int32_t h = 0x1ED980DD; // Random

    for (size_t i = 0; i < s.size(); ++i) {
        auto tmp = static_cast<int32_t>(s[i]);
        tmp = tmp << (8 * (i % 4));
        h = h ^ tmp;
    }

    return h;
}
