#pragma once
#include <m2/math/primitives/Exact.h>

namespace m2 {
	struct RectF;

	struct RectE {
		Exact x, y, w, h;

		// Constructors

		RectE() = default;
		RectE(const Exact x, const Exact y, const Exact w, const Exact h) : x(x), y(y), w(w), h(h) {}
		RectE(const int x, const int y, const int w, const int h) : x(x), y(y), w(w), h(h) {}

		// Operators

		bool operator==(const RectE& other) const { return x == other.x && y == other.y && w == other.w && h == other.h; }
		explicit operator bool() const { return Exact::Zero() < w && Exact::Zero() < h; }
		explicit operator RectF() const;

		// Accessors

		[[nodiscard]] Exact GetX2() const { return x + w; }
		[[nodiscard]] Exact GetY2() const { return y + h; }
		[[nodiscard]] Exact GetXCenter() const { return x + w / Exact{2}; }
		[[nodiscard]] Exact GetYCenter() const { return y + h / Exact{2}; }
	};
}
