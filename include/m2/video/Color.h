#pragma once
#include <cstdint>
#include "../Meta.h"
#include <Sprite.pb.h>
#include <box2d/b2_draw.h>
#include <SDL2/SDL_pixels.h>

namespace m2 {
	struct RGB {
		uint8_t r{}, g{}, b{};

		RGB operator*(const float rhs) const { return RGB{RoundU8(F(r) * rhs), RoundU8(F(g) * rhs), RoundU8(F(b) * rhs)}; }
		RGB operator/(const float rhs) const { return RGB{RoundU8(F(r) / rhs), RoundU8(F(g) / rhs), RoundU8(F(b) / rhs)}; }
	};

	struct RGBA {
		uint8_t r{}, g{}, b{}, a{};

		RGBA(const int _r, const int _g, const int _b, const int _a) : r(U8(_r)), g(U8(_g)), b(U8(_b)), a(U8(_a)) {}
		RGBA(const uint8_t _r, const uint8_t _g, const uint8_t _b, const uint8_t _a) : r(_r), g(_g), b(_b), a(_a) {}
		RGBA(const float _r, const float _g, const float _b, const float _a) : r(RoundU8(_r * 255.0f)), g(RoundU8(_g * 255.0f)), b(RoundU8(_b * 255.0f)), a(RoundU8(_a * 255.0f)) {}
		explicit RGBA(const pb::Color& c) : r(static_cast<uint8_t>(c.r())), g(static_cast<uint8_t>(c.g())), b(static_cast<uint8_t>(c.b())), a(static_cast<uint8_t>(c.a())) {}
		explicit RGBA(const SDL_Color& c) : r(c.r), g(c.g), b(c.b), a(c.a) {}
		explicit RGBA(const b2Color& c) : RGBA(c.r, c.g, c.b, c.a) {}
		explicit operator RGB() const { return RGB{r, g, b}; }
		explicit operator SDL_Color() const { return SDL_Color{r, g, b, a}; }

		/// Alpha value isn't scaled
		RGBA operator*(const float rhs) const { return RGBA{RoundU8(F(r) * rhs), RoundU8(F(g) * rhs), RoundU8(F(b) * rhs), a}; }
		/// Alpha value isn't scaled
		RGBA operator/(const float rhs) const { return RGBA{RoundU8(F(r) / rhs), RoundU8(F(g) / rhs), RoundU8(F(b) / rhs), a}; }

		static RGBA White;
		static RGBA Red;
		static RGBA Yellow;
		static RGBA Green;
		static RGBA Cyan;
		static RGBA Blue;
		static RGBA Magenta;
		static RGBA Black;

		static RGBA TransparentWhite;
		static RGBA TransparentRed;
		static RGBA TransparentYellow;
		static RGBA TransparentGreen;
		static RGBA TransparentCyan;
		static RGBA TransparentBlue;
		static RGBA TransparentMagenta;
		static RGBA TransparentBlack;
	};
}
