#pragma once
#include <m2/math/primitives/Exact.h>
#include <cstdint>

namespace m2 {
	class Rng {
	public:
		virtual ~Rng() = default;

		/// Generates random bits
		virtual uint64_t GenerateNextNumber64() = 0;
		/// Generates numbers in [Exact::Min, Exact::Max]
		virtual Exact GenerateNextExact() = 0;
		/// Generates numbers in [0, 1]
		virtual Exact GenerateNextNormalizedExact() = 0;
		/// Generates numbers in [-1, 1]
		virtual Exact GenerateNextFractionalExact() = 0;
	};
}
