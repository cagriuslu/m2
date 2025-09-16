#pragma once
#include <m2/math/primitives/Exact.h>
#include <m2/math/primitives/Float.h>
#include <m2/ProxyTypes.h>

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

		VecE operator-(const VecE& rhs) const { return {_x - rhs.GetX(), _y - rhs.GetY()}; }
		explicit operator VecF() const;

		// Accessors

		[[nodiscard]] const Exact& GetX() const { return _x; }
		[[nodiscard]] const Exact& GetY() const { return _y; }

		// Modifiers

		[[nodiscard]] Exact GetLengthSquaredFE() const;
		[[nodiscard]] Exact GetDistanceToSquaredFE(const VecE& other) const;
	};
}
