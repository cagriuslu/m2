#pragma once
#include <m2/ProxyTypes.h>
#include <ProxyTypes.pb.h>

namespace m2 {
	struct VecI;
	struct VecF;

	class VecFF {
		FF _x, _y;

	public:
		VecFF() = default;
		VecFF(FF x, FF y) : _x(std::move(x)), _y(std::move(y)) {}
		VecFF(const FF& x, const FF& y) : _x(x), _y(y) {}
		VecFF(FF&& x, FF&& y) : _x(std::move(x)), _y(std::move(y)) {}
		VecFF(const int x, const int y) : _x(x), _y(y) {}
		explicit VecFF(const VecI&);
		explicit VecFF(const pb::VecI&);
		explicit VecFF(const pb::VecFF&);

		// Operators

		explicit operator bool() const { return not _x.IsZero() || not _y.IsZero(); }
		/// This operation isn't deterministic
		explicit operator VecF() const;

		// Accessors

		const FF& GetX() const { return _x; }
		const FF& GetY() const { return _y; }
	};
}
