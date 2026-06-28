#include <m2/common/math/Rational.h>
#include <m2/common/Constants.h>
#include <m2/common/Error.h>
#include <numeric>
#include <cinttypes>
#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace m2::internal {

int64_t UnsafeMultiplyWithBoundsCheck(const int64_t a, const int64_t b) {
	int64_t result;
#if defined(_MSC_VER)
	result = a * b;
	const int64_t high = __mulh(a, b);
	if (high != (result >> 63)) {
		throw M2_ERROR("Rational arithmetic overflow");
	}
#else
	if (__builtin_mul_overflow(a, b, &result)) {
		throw M2_ERROR("Rational arithmetic overflow");
	}
#endif
	return result;
}

int64_t UnsafeAddWithBoundsCheck(const int64_t a, const int64_t b) {
	int64_t result;
#if defined(_MSC_VER)
	if ((b > 0 && a > INT64_MAX - b) || (b < 0 && a < INT64_MIN - b)) {
		throw M2_ERROR("Rational arithmetic overflow");
	}
	result = a + b;
#else
	if (__builtin_add_overflow(a, b, &result)) {
		throw M2_ERROR("Rational arithmetic overflow");
	}
#endif
	return result;
}

m2::Rational simplify(int64_t n, int64_t d) {
	auto gcd = std::gcd(n, d);
	if (gcd) {
		return m2::Rational{n / gcd, d / gcd};
	} else {
		return {};
	}
}

}

// Default protobuf object has n=0,d=0. If that's the case, create Rational{0,1}
m2::Rational::Rational(const pb::Rational& r) : _n(r.n()), _d(!r.n() && !r.d() ? 1 : r.d()) {}
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

	const auto raised_d = round(d * static_cast<double>(precision));
	const auto raised = static_cast<int64_t>(raised_d);
	*this = internal::simplify(raised, precision);
}

m2::Rational m2::Rational::Simplify() const {
	return internal::simplify(_n, _d);
}
m2::Rational m2::Rational::Mod(const Rational& other) const {
	auto lhs = internal::UnsafeMultiplyWithBoundsCheck(_n, other._d);
	auto rhs = internal::UnsafeMultiplyWithBoundsCheck(_d, other._n);
	auto mod_result = internal::UnsafeAddWithBoundsCheck(lhs % rhs, rhs) % rhs;
	return internal::simplify(mod_result, internal::UnsafeMultiplyWithBoundsCheck(_d, other._d));
}

m2::Rational m2::Rational::operator+(const Rational& rhs) const {
	int64_t n, d;
	if (_d == rhs._d) {
		n = internal::UnsafeAddWithBoundsCheck(_n, rhs._n);
		d = _d;
	} else {
		n = internal::UnsafeAddWithBoundsCheck(internal::UnsafeMultiplyWithBoundsCheck(_n, rhs._d), internal::UnsafeMultiplyWithBoundsCheck(rhs._n, _d));
		d = internal::UnsafeMultiplyWithBoundsCheck(_d, rhs._d);
	}
	return internal::simplify(n, d);
}
m2::Rational& m2::Rational::operator+=(const Rational& rhs) {
	*this = *this + rhs;
	return *this;
}
m2::Rational m2::Rational::operator*(const Rational& rhs) const {
	return internal::simplify(internal::UnsafeMultiplyWithBoundsCheck(_n, rhs._n), internal::UnsafeMultiplyWithBoundsCheck(_d, rhs._d));
}
m2::Rational m2::Rational::operator*(int64_t rhs) const {
	return internal::simplify(internal::UnsafeMultiplyWithBoundsCheck(_n, rhs), _d);
}
m2::Rational m2::Rational::operator/(const Rational& rhs) const {
	return internal::simplify(internal::UnsafeMultiplyWithBoundsCheck(_n, rhs._d), internal::UnsafeMultiplyWithBoundsCheck(_d, rhs._n));
}
m2::Rational m2::Rational::operator/(int64_t rhs) const {
	return internal::simplify(_n, internal::UnsafeMultiplyWithBoundsCheck(_d, rhs));
}

m2::Rational m2::Rational::PiMul2() {
	return Rational{PI_MUL2};
}

bool operator==(const m2::Rational& lhs, const m2::Rational& rhs) {
	if (lhs.GetD() == rhs.GetD()) {
		return lhs.GetN() == rhs.GetN();
	} else {
		auto lhs_n = m2::internal::UnsafeMultiplyWithBoundsCheck(lhs.GetN(), rhs.GetD());
		auto rhs_n = m2::internal::UnsafeMultiplyWithBoundsCheck(rhs.GetN(), lhs.GetD());
		return lhs_n == rhs_n;
	}
}
bool operator!=(const m2::Rational& lhs, const m2::Rational& rhs) {
	return !(lhs == rhs);
}
bool operator<(const m2::Rational& lhs, const m2::Rational& rhs) {
	if (lhs.GetD() == rhs.GetD()) {
		return lhs.GetN() < rhs.GetN();
	} else {
		auto lhs_n = m2::internal::UnsafeMultiplyWithBoundsCheck(lhs.GetN(), rhs.GetD());
		auto rhs_n = m2::internal::UnsafeMultiplyWithBoundsCheck(rhs.GetN(), lhs.GetD());
		return lhs_n < rhs_n;
	}
}
bool operator>(const m2::Rational& lhs, const m2::Rational& rhs) {
	if (lhs.GetD() == rhs.GetD()) {
		return lhs.GetN() > rhs.GetN();
	} else {
		auto lhs_n = m2::internal::UnsafeMultiplyWithBoundsCheck(lhs.GetN(), rhs.GetD());
		auto rhs_n = m2::internal::UnsafeMultiplyWithBoundsCheck(rhs.GetN(), lhs.GetD());
		return lhs_n > rhs_n;
	}
}
bool operator<=(const m2::Rational& lhs, const m2::Rational& rhs) {
	if (lhs.GetD() == rhs.GetD()) {
		return lhs.GetN() <= rhs.GetN();
	} else {
		auto lhs_n = m2::internal::UnsafeMultiplyWithBoundsCheck(lhs.GetN(), rhs.GetD());
		auto rhs_n = m2::internal::UnsafeMultiplyWithBoundsCheck(rhs.GetN(), lhs.GetD());
		return lhs_n <= rhs_n;
	}
}
bool operator>=(const m2::Rational& lhs, const m2::Rational& rhs) {
	if (lhs.GetD() == rhs.GetD()) {
		return lhs.GetN() >= rhs.GetN();
	} else {
		auto lhs_n = m2::internal::UnsafeMultiplyWithBoundsCheck(lhs.GetN(), rhs.GetD());
		auto rhs_n = m2::internal::UnsafeMultiplyWithBoundsCheck(rhs.GetN(), lhs.GetD());
		return lhs_n >= rhs_n;
	}
}

auto std::formatter<m2::Rational>::format(const m2::Rational& r, std::format_context& ctx) const -> std::format_context::iterator {
	char buffer[20 + 1 + 20 + 1];
	snprintf(buffer, sizeof(buffer), "%" PRId64 "/%" PRId64, r.GetN(), r.GetD());
	return std::formatter<std::string>::format(buffer, ctx);
}
