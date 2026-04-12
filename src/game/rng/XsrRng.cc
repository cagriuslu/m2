#include <m2/game/rng/XsrRng.h>
#include <m2/math/Bit.h>
#include <m2/Meta.h>
#include <array>

m2::XsrRng::XsrRng(const uint64_t seed1, const uint64_t seed2, const uint64_t seed3, const uint64_t seed4) : Rng(), _xoshiro256ppState() {
	_xoshiro256ppState[0] = seed1;
	_xoshiro256ppState[1] = seed2;
	_xoshiro256ppState[2] = seed3;
	_xoshiro256ppState[3] = seed4;
}

void m2::XsrRng::GenerateNextNumber64(uint64_t& out) {
	// xoshiro256++ algorithm

	const auto result = RotateLeft64(_xoshiro256ppState[0] + _xoshiro256ppState[3], 23) + _xoshiro256ppState[0];
	const auto tmp = _xoshiro256ppState[1] << 17;
	_xoshiro256ppState[2] ^= _xoshiro256ppState[0];
	_xoshiro256ppState[3] ^= _xoshiro256ppState[1];
	_xoshiro256ppState[1] ^= _xoshiro256ppState[2];
	_xoshiro256ppState[0] ^= _xoshiro256ppState[3];
	_xoshiro256ppState[2] ^= tmp;
	_xoshiro256ppState[3] = RotateLeft64(_xoshiro256ppState[3], 45);
	out = result;
}

void m2::XsrRng::GenerateNextExact(Exact& out) {
	// It's recommended that the upper 53 bits is used for fractional numbers
	uint64_t nextNumber;
	GenerateNextNumber64(nextNumber);
	const auto onlyHigherBits = nextNumber >> (64 - 53);
	out = Exact{std::in_place, static_cast<int32_t>(onlyHigherBits)};
}

void m2::XsrRng::GenerateNextNormalizedExact(Exact& out) {
	uint64_t nextNumber;
	GenerateNextNumber64(nextNumber);
	const auto onlyHigherBits = nextNumber >> (64 - Exact::PRECISION);
	out = Exact{std::in_place, static_cast<int32_t>(onlyHigherBits)};
}

void m2::XsrRng::GenerateNextFractionalExact(Exact& out) {
	Exact normalized;
	GenerateNextNormalizedExact(normalized);
	out = normalized * Exact{2} - Exact{1};
}
