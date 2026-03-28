#pragma once
#include <m2/math/primitives/Exact.h>
#include <cstdint>

namespace m2 {
	class Rng {
	public:
		virtual ~Rng() = default;

		/// Following functions return void by design. This ensures that they aren't called from function parameters
		/// (ex. Func(GenerateNextNumber(), GenerateNextNumber()) because different platforms may call such these
		/// functions in different order, which could break the determinism of the RNG.

		/// Generates random bits
		virtual void GenerateNextNumber64(uint64_t& out) = 0;
		/// Generates numbers in [Exact::Min, Exact::Max]
		virtual void GenerateNextExact(Exact& out) = 0;
		/// Generates numbers in [0, 1]
		virtual void GenerateNextNormalizedExact(Exact& out) = 0;
		/// Generates numbers in [-1, 1]
		virtual void GenerateNextFractionalExact(Exact& out) = 0;
	};
}
