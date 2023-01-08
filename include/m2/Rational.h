#ifndef M2_RATIONAL_H
#define M2_RATIONAL_H

#include <Rational.pb.h>

namespace m2 {
	class Rational {
		int32_t _n{}, _d{1};

	public:
		inline Rational() = default;
		inline explicit Rational(int i) : _n(i) {}
		inline explicit Rational(int32_t n, int32_t d) : _n(n), _d(d) {}
		inline explicit Rational(const pb::Rational& r) : _n(r.n()), _d(r.d() == 0 ? 1 : r.d()) {}
		explicit Rational(float f);

		Rational operator+(const Rational& rhs) const;
		Rational operator*(int32_t rhs) const;
		Rational operator/(const Rational& rhs) const;

		[[nodiscard]] Rational mod(const Rational& other) const;

		[[nodiscard]] inline int32_t n() const { return _n; }
		[[nodiscard]] inline int32_t d() const { return _d; }
		[[nodiscard]] inline pb::Rational to_pb() const { pb::Rational r; r.set_n(_n); r.set_d(_d); return r; }

		[[nodiscard]] Rational simplify() const;
	};

	inline pb::Rational make_rational(int32_t n = 0, int32_t d = 1) { pb::Rational r; r.set_n(n); r.set_d(d); return r; }

	inline float to_float(const pb::Rational& r) {
		return static_cast<float>(r.n()) / static_cast<float>(r.d());
	}
}

bool operator==(const m2::Rational& lhs, const m2::Rational& rhs);
bool operator!=(const m2::Rational& lhs, const m2::Rational& rhs);
bool operator<(const m2::Rational& lhs, const m2::Rational& rhs);
bool operator>(const m2::Rational& lhs, const m2::Rational& rhs);
bool operator<=(const m2::Rational& lhs, const m2::Rational& rhs);
bool operator>=(const m2::Rational& lhs, const m2::Rational& rhs);

#endif //M2_RATIONAL_H
