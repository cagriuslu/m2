#include <m2/Rational.h>
#include <numeric>

namespace m2::internal {

m2::Rational simplify(int64_t n, int64_t d) {
	auto gcd = std::gcd(n, d);
	if (gcd) {
		return m2::Rational{n / gcd, d / gcd};
	} else {
		return {};
	}
}

}

m2::Rational::Rational(double d) {
	int64_t precision;
	{
		auto abs_f = fabs(d);
		// Decide on the precision
		// It's wrong to calculate precision calculation based on decimal orders.
		// But most float number is entered as a decimal number.
		// Correct precision calculation is below.
		if (100000000000000.0f < abs_f) {
			precision = 1LL;
		} else if (10000000000000.0f < abs_f) {
			precision = 10LL;
		} else if (1000000000000.0f < abs_f) {
			precision = 100LL;
		} else if (100000000000.0f < abs_f) {
			precision = 1000LL;
		} else if (10000000000.0f < abs_f) {
			precision = 10000LL;
		} else if (1000000000.0f < abs_f) {
			precision = 100000LL;
		} else if (100000000.0f < abs_f) {
			precision = 1000000LL;
		} else if (10000000.0f < abs_f) {
			precision = 10000000LL;
		} else if (1000000.0f < abs_f) {
			precision = 100000000LL;
		} else if (100000.0f < abs_f) {
			precision = 1000000000LL;
		} else if (10000.0f < abs_f) {
			precision = 10000000000LL;
		} else if (1000.0f < abs_f) {
			precision = 100000000000LL;
		} else if (100.0f < abs_f) {
			precision = 1000000000000LL;
		} else if (10.0f < abs_f) {
			precision = 10000000000000LL;
		} else if (1.0f < abs_f) {
			precision = 100000000000000LL;
		} else {
			precision = 1000000000000000LL;
		}
		
//		int exp;
//		frexpf(f, &exp);
//		if (exp < 0) {
//			precision = (1LL << 23);
//		} else if (23 < exp) {
//			precision = 1LL;
//		} else {
//			precision = (1LL << (23 - exp));
//		}
	}

	auto raised_d = round(d * (double) precision);
	auto raised = (int64_t) raised_d;

	*this = internal::simplify(raised, precision);
}

m2::Rational m2::Rational::operator+(const Rational& rhs) const {
	int64_t n, d;
	if (_d == rhs._d) {
		n = _n + rhs._n;
		d = _d;
	} else {
		n = _n * rhs._d + rhs._n * _d;
		d = _d * rhs._d;
	}
	return internal::simplify(n, d);
}

m2::Rational m2::Rational::operator*(int64_t rhs) const {
	return internal::simplify(_n * rhs, _d);
}

m2::Rational m2::Rational::operator/(const Rational& rhs) const {
	return internal::simplify(_n * rhs._d, _d * rhs._n);
}

m2::Rational m2::Rational::mod(const Rational& other) const {
	auto lhs = _n * other._d;
	auto rhs = _d * other._n;
	auto mod_result = lhs % rhs;
	return internal::simplify(mod_result, _d * other._d);
}

[[nodiscard]] m2::Rational m2::Rational::simplify() const {
	return internal::simplify(_n, _d);
}

bool operator==(const m2::Rational& lhs, const m2::Rational& rhs) {
	if (lhs.d() == rhs.d()) {
		return lhs.n() == rhs.n();
	} else {
		auto lhs_n = lhs.n() * rhs.d();
		auto rhs_n = rhs.n() * lhs.d();
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
		auto lhs_n = lhs.n() * rhs.d();
		auto rhs_n = rhs.n() * lhs.d();
		return lhs_n < rhs_n;
	}
}
bool operator>(const m2::Rational& lhs, const m2::Rational& rhs) {
	if (lhs.d() == rhs.d()) {
		return lhs.n() > rhs.n();
	} else {
		auto lhs_n = lhs.n() * rhs.d();
		auto rhs_n = rhs.n() * lhs.d();
		return lhs_n > rhs_n;
	}
}
bool operator<=(const m2::Rational& lhs, const m2::Rational& rhs) {
	if (lhs.d() == rhs.d()) {
		return lhs.n() <= rhs.n();
	} else {
		auto lhs_n = lhs.n() * rhs.d();
		auto rhs_n = rhs.n() * lhs.d();
		return lhs_n <= rhs_n;
	}
}
bool operator>=(const m2::Rational& lhs, const m2::Rational& rhs) {
	if (lhs.d() == rhs.d()) {
		return lhs.n() >= rhs.n();
	} else {
		auto lhs_n = lhs.n() * rhs.d();
		auto rhs_n = rhs.n() * lhs.d();
		return lhs_n >= rhs_n;
	}
}
