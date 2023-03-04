#ifndef M2_RATIONAL_H
#define M2_RATIONAL_H

#include <Rational.pb.h>

namespace m2 {
	class Rational {
		int64_t _n{}, _d{1};

	public:
		inline Rational() = default;
		inline Rational(int64_t n, int64_t d) : _n(n), _d(d) {}
		inline explicit Rational(const pb::Rational& r) : _n(r.n()), _d(r.d()) {}
		explicit Rational(double d);

		Rational operator+(const Rational& rhs) const;
		Rational operator*(int64_t rhs) const;
		Rational operator/(const Rational& rhs) const;

		[[nodiscard]] Rational mod(const Rational& other) const;

		[[nodiscard]] inline int64_t n() const { return _n; }
		[[nodiscard]] inline int64_t d() const { return _d; }
		[[nodiscard]] inline pb::Rational to_pb() const { pb::Rational r; r.set_n(_n); r.set_d(_d); return r; }

		[[nodiscard]] Rational simplify() const;
	};

	inline float to_float(const pb::Rational& r) {
		return static_cast<float>(r.n()) / static_cast<float>(r.d());
	}
	inline double to_double(const pb::Rational& r) {
		return static_cast<double>(r.n()) / static_cast<double>(r.d());
	}
}

bool operator==(const m2::Rational& lhs, const m2::Rational& rhs);
bool operator!=(const m2::Rational& lhs, const m2::Rational& rhs);
bool operator<(const m2::Rational& lhs, const m2::Rational& rhs);
bool operator>(const m2::Rational& lhs, const m2::Rational& rhs);
bool operator<=(const m2::Rational& lhs, const m2::Rational& rhs);
bool operator>=(const m2::Rational& lhs, const m2::Rational& rhs);

#endif //M2_RATIONAL_H
