#pragma once
#include <Rational.pb.h>

namespace m2 {
	class Rational {
		int64_t _n{0}, _d{1};

	public:
		constexpr Rational() = default;
		constexpr Rational(const int64_t n, const int64_t d) : _n(n), _d(d) {}
		explicit Rational(const pb::Rational& r);
		explicit Rational(double d);

		Rational operator+(const Rational& rhs) const;
		Rational& operator+=(const Rational& rhs);
		Rational operator*(const Rational& rhs) const;
		Rational operator*(int64_t rhs) const;
		Rational operator/(const Rational& rhs) const;
		Rational operator/(int64_t rhs) const;
		explicit operator bool() const { return _n; }

		[[nodiscard]] Rational Simplify() const;
		[[nodiscard]] Rational Mod(const Rational& other) const;

		[[nodiscard]] int64_t GetN() const { return _n; }
		[[nodiscard]] int64_t GetD() const { return _d; }
		[[nodiscard]] float ToFloat() const { return static_cast<float>(_n) / static_cast<float>(_d); }
		[[nodiscard]] double ToDouble() const { return static_cast<double>(_n) / static_cast<double>(_d); }
		[[nodiscard]] pb::Rational ToPb() const { pb::Rational r; r.set_n(_n); r.set_d(_d); return r; }

		static Rational Zero() { return Rational{}; }
		static Rational One() { return Rational{1,1}; }
		static Rational PiMul2();
	};
	std::string ToString(const Rational& r);

	inline float ToFloat(const pb::Rational& r) {
		return static_cast<float>(r.n()) / static_cast<float>(r.d());
	}
	inline double ToDouble(const pb::Rational& r) {
		return static_cast<double>(r.n()) / static_cast<double>(r.d());
	}
}

bool operator==(const m2::Rational& lhs, const m2::Rational& rhs);
bool operator!=(const m2::Rational& lhs, const m2::Rational& rhs);
bool operator<(const m2::Rational& lhs, const m2::Rational& rhs);
bool operator>(const m2::Rational& lhs, const m2::Rational& rhs);
bool operator<=(const m2::Rational& lhs, const m2::Rational& rhs);
bool operator>=(const m2::Rational& lhs, const m2::Rational& rhs);
