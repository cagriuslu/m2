#include <m2/game/rng/XsrRng.h>
#include <m2/detail/Bit.h>
#include <m2/Meta.h>
#include <array>

m2::XsrRng::XsrRng(const uint64_t seed1, const uint64_t seed2, const uint64_t seed3, const uint64_t seed4) : Rng(), _xoshiro256ppState() {
	_xoshiro256ppState[0] = seed1;
	_xoshiro256ppState[1] = seed2;
	_xoshiro256ppState[2] = seed3;
	_xoshiro256ppState[3] = seed4;
}

uint64_t m2::XsrRng::GenerateNextNumber64() {
	// xoshiro256++ algorithm

	const auto result = RotateLeft64(_xoshiro256ppState[0] + _xoshiro256ppState[3], 23) + _xoshiro256ppState[0];
	const auto tmp = _xoshiro256ppState[1] << 17;
	_xoshiro256ppState[2] ^= _xoshiro256ppState[0];
	_xoshiro256ppState[3] ^= _xoshiro256ppState[1];
	_xoshiro256ppState[1] ^= _xoshiro256ppState[2];
	_xoshiro256ppState[0] ^= _xoshiro256ppState[3];
	_xoshiro256ppState[2] ^= tmp;
	_xoshiro256ppState[3] = RotateLeft64(_xoshiro256ppState[3], 45);
	return result;
}

m2::Exact m2::XsrRng::GenerateNextNumberExact() {
	// It's recommended that the upper 53 bits is used for fractional numbers
	const auto nextNumber = GenerateNextNumber64();
	const auto onlyHigherBits = nextNumber >> (64 - 53);
	const auto int32 = static_cast<int32_t>(onlyHigherBits);
	return Exact{std::in_place, int32};
}
