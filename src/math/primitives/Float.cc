#include <m2/math/primitives/Float.h>
#include <array>
#include <algorithm>

std::string m2::Float::ToString() const {
	std::array<char, 32> buffer{};
	std::ranges::fill(buffer, 0);
	snprintf(buffer.data(), buffer.size(), "%+014.06f", ToFloat());
	return {buffer.data()};
}
