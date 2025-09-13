#pragma once
#include <m2/math/primitives/Exact.h>
#include <cstdint>

namespace m2 {
	class Rng {
	public:
		virtual ~Rng() = default;

		virtual uint64_t GenerateNextNumber64() = 0;
		virtual Exact GenerateNextNumberExact() = 0;
	};
}
