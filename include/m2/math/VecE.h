#pragma once
#include <m2/math/primitives/Exact.h>

namespace m2 {
	class VecF;

	class VecE {
		Exact _x, _y;

	public:
		// Constructors

		VecE() = default;
		VecE(const Exact& x, const Exact& y) : _x(x), _y(y) {}
		VecE(Exact&& x, Exact&& y) : _x(std::move(x)), _y(std::move(y)) {}
		explicit VecE(const VecF&);

		// Operators

		explicit operator VecF() const;
	};
}
