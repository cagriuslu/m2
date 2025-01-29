#pragma once
#include <cstdint>
#include "../Meta.h"
#include <Sprite.pb.h>

namespace m2 {
	struct RGB {
		uint8_t r{}, g{}, b{};

		RGB operator*(const float rhs) const { return RGB{u8round(F(r) * rhs), u8round(F(g) * rhs), u8round(F(b) * rhs)}; }
		RGB operator/(const float rhs) const { return RGB{u8round(F(r) / rhs), u8round(F(g) / rhs), u8round(F(b) / rhs)}; }
	};

	struct RGBA {
		uint8_t r{}, g{}, b{}, a{};

		RGBA(const uint8_t _r, const uint8_t _g, const uint8_t _b, const uint8_t _a) : r(_r), g(_g), b(_b), a(_a) {}
		explicit RGBA(const pb::Color& c) : r(static_cast<uint8_t>(c.r())), g(static_cast<uint8_t>(c.g())), b(static_cast<uint8_t>(c.b())), a(static_cast<uint8_t>(c.a())) {}
		explicit operator RGB() const { return RGB{r, g, b}; }

		/// Alpha value isn't scaled
		RGBA operator*(const float rhs) const { return RGBA{u8round(F(r) * rhs), u8round(F(g) * rhs), u8round(F(b) * rhs), a}; }
		/// Alpha value isn't scaled
		RGBA operator/(const float rhs) const { return RGBA{u8round(F(r) / rhs), u8round(F(g) / rhs), u8round(F(b) / rhs), a}; }
	};
}
