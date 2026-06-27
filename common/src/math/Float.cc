#include <m2/common/math/Float.h>
#include <array>
#include <algorithm>

std::string m2::Float::ToString() const {
	std::array<char, 32> buffer{};
	std::ranges::fill(buffer, 0);
	snprintf(buffer.data(), buffer.size(), "%+014.06f", ToFloat());
	return {buffer.data()};
}

auto std::formatter<m2::Float>::format(const m2::Float& value, std::format_context& ctx) const -> std::format_context::iterator {
	return std::formatter<std::string>::format(value.ToString(), ctx);
}
