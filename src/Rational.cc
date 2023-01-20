#include <m2/Rational.h>
#include <numeric>

namespace m2::internal {

m2::Rational simplify(int64_t n, int64_t d) {
	auto gcd = std::gcd(n, d);
	if (gcd) {
		return m2::Rational{static_cast<int32_t>(n / gcd), static_cast<int32_t>(d / gcd)};
	} else {
		return {};
	}
}

}

m2::Rational::Rational(float f) {
	int64_t precision;
	{
		auto abs_f = fabsf(f);
		// Decide on the precision
		if (1000000.0f < abs_f) {
			precision = 1LL;
		} else if (100000.0f < abs_f) {
			precision = 10LL;
		} else if (10000.0f < abs_f) {
			precision = 100LL;
		} else if (1000.0f < abs_f) {
			precision = 1000LL;
		} else if (100.0f < abs_f) {
			precision = 10000LL;
		} else if (10.0f < abs_f) {
			precision = 100000LL;
		} else if (1.0f < abs_f) {
			precision = 1000000LL;
		} else {
			precision = 10000000LL;
		}
	}

	auto raised_f = roundf(f * (float) precision);
	auto raised = (int64_t) raised_f;

	*this = internal::simplify(raised, precision);
}

m2::Rational m2::Rational::operator+(const Rational& rhs) const {
	int64_t n, d;
	if (_d == rhs._d) {
		n = (int64_t) _n + (int64_t) rhs._n;
		d = _d;
	} else {
		n = (int64_t) _n * (int64_t) rhs._d + (int64_t) rhs._n * (int64_t) _d;
		d = (int64_t) _d * (int64_t) rhs._d;
	}
	return internal::simplify(n, d);
}

m2::Rational m2::Rational::operator*(int32_t rhs) const {
	return internal::simplify((int64_t) _n * (int64_t) rhs, _d);
}

m2::Rational m2::Rational::operator/(const Rational& rhs) const {
	return internal::simplify((int64_t) _n * (int64_t) rhs._d, (int64_t) _d * (int64_t) rhs._n);
}

m2::Rational m2::Rational::mod(const Rational& other) const {
	int64_t lhs = (int64_t) _n * (int64_t) other._d;
	int64_t rhs = (int64_t) _d * (int64_t) other._n;
	int64_t mod_result = lhs % rhs;
	return internal::simplify(mod_result, (int64_t) _d * (int64_t) other._d);
}

[[nodiscard]] m2::Rational m2::Rational::simplify() const {
	return internal::simplify(_n, _d);
}

bool operator==(const m2::Rational& lhs, const m2::Rational& rhs) {
	if (lhs.d() == rhs.d()) {
		return lhs.n() == rhs.n();
	} else {
		int64_t lhs_n = (int64_t) lhs.n() * (int64_t) rhs.d();
		int64_t rhs_n = (int64_t) rhs.n() * (int64_t) lhs.d();
		return lhs_n == rhs_n;
	}
}
bool operator!=(const m2::Rational& lhs, const m2::Rational& rhs) {
	return !(lhs == rhs);
}
bool operator<(const m2::Rational& lhs, const m2::Rational& rhs) {
	if (lhs.d() == rhs.d()) {
		return lhs.n() < rhs.n();
	} else {
		int64_t lhs_n = (int64_t) lhs.n() * (int64_t) rhs.d();
		int64_t rhs_n = (int64_t) rhs.n() * (int64_t) lhs.d();
		return lhs_n < rhs_n;
	}
}
bool operator>(const m2::Rational& lhs, const m2::Rational& rhs) {
	if (lhs.d() == rhs.d()) {
		return lhs.n() > rhs.n();
	} else {
		int64_t lhs_n = (int64_t) lhs.n() * (int64_t) rhs.d();
		int64_t rhs_n = (int64_t) rhs.n() * (int64_t) lhs.d();
		return lhs_n > rhs_n;
	}
}
bool operator<=(const m2::Rational& lhs, const m2::Rational& rhs) {
	if (lhs.d() == rhs.d()) {
		return lhs.n() <= rhs.n();
	} else {
		int64_t lhs_n = (int64_t) lhs.n() * (int64_t) rhs.d();
		int64_t rhs_n = (int64_t) rhs.n() * (int64_t) lhs.d();
		return lhs_n <= rhs_n;
	}
}
bool operator>=(const m2::Rational& lhs, const m2::Rational& rhs) {
	if (lhs.d() == rhs.d()) {
		return lhs.n() >= rhs.n();
	} else {
		int64_t lhs_n = (int64_t) lhs.n() * (int64_t) rhs.d();
		int64_t rhs_n = (int64_t) rhs.n() * (int64_t) lhs.d();
		return lhs_n >= rhs_n;
	}
}
