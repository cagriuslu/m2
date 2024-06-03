#pragma once
#include <cstdint>
#include "Meta.h"

namespace m2 {
	struct RGB {
		uint8_t r{}, g{}, b{};

		RGB operator/(float rhs) const { return RGB{u8round(F(r) / rhs), u8round(F(g) / rhs), u8round(F(b) / rhs)}; }
	};

	struct RGBA {
		uint8_t r{}, g{}, b{}, a{};
	};
}  // namespace m2
