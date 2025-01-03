#pragma once
#include <Rational.pb.h>

namespace m2 {
	class Rational {
		int64_t _n{0}, _d{1};

	public:
		constexpr Rational() = default;
		constexpr Rational(int64_t n, int64_t d) : _n(n), _d(d) {}
		explicit Rational(const pb::Rational& r);
		explicit Rational(double d);

		Rational operator+(const Rational& rhs) const;
		Rational& operator+=(const Rational& rhs);
		Rational operator*(const Rational& rhs) const;
		Rational operator*(int64_t rhs) const;
		Rational operator/(const Rational& rhs) const;
		Rational operator/(int64_t rhs) const;

		[[nodiscard]] Rational simplify() const;
		[[nodiscard]] Rational mod(const Rational& other) const;

		[[nodiscard]] int64_t n() const { return _n; }
		[[nodiscard]] int64_t d() const { return _d; }
		[[nodiscard]] float to_float() const { return static_cast<float>(_n) / static_cast<float>(_d); }
		[[nodiscard]] double to_double() const { return static_cast<double>(_n) / static_cast<double>(_d); }
		[[nodiscard]] pb::Rational to_pb() const { pb::Rational r; r.set_n(_n); r.set_d(_d); return r; }

		static Rational zero() { return Rational{}; }
		static Rational one() { return Rational{1,1}; }
		static Rational pi_mul2();
	};
	std::string ToString(const Rational& r);

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
