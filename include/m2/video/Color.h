#pragma once
#include <cstdint>
#include "../Meta.h"
#include <Sprite.pb.h>

namespace m2 {
	struct RGB {
		uint8_t r{}, g{}, b{};

		RGB operator*(const float rhs) const { return RGB{RoundU8(F(r) * rhs), RoundU8(F(g) * rhs), RoundU8(F(b) * rhs)}; }
		RGB operator/(const float rhs) const { return RGB{RoundU8(F(r) / rhs), RoundU8(F(g) / rhs), RoundU8(F(b) / rhs)}; }
	};

	struct RGBA {
		uint8_t r{}, g{}, b{}, a{};

		RGBA(const uint8_t _r, const uint8_t _g, const uint8_t _b, const uint8_t _a) : r(_r), g(_g), b(_b), a(_a) {}
		explicit RGBA(const pb::Color& c) : r(static_cast<uint8_t>(c.r())), g(static_cast<uint8_t>(c.g())), b(static_cast<uint8_t>(c.b())), a(static_cast<uint8_t>(c.a())) {}
		explicit RGBA(const SDL_Color& c) : r(c.r), g(c.g), b(c.b), a(c.a) {}
		explicit operator RGB() const { return RGB{r, g, b}; }
		explicit operator SDL_Color() const { return SDL_Color{r, g, b, a}; }

		/// Alpha value isn't scaled
		RGBA operator*(const float rhs) const { return RGBA{RoundU8(F(r) * rhs), RoundU8(F(g) * rhs), RoundU8(F(b) * rhs), a}; }
		/// Alpha value isn't scaled
		RGBA operator/(const float rhs) const { return RGBA{RoundU8(F(r) / rhs), RoundU8(F(g) / rhs), RoundU8(F(b) / rhs), a}; }
	};
}
