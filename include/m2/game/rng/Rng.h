#pragma once
#include <m2/math/primitives/Fixed.h>
#include <cstdint>

namespace m2 {
	class Rng {
	public:
		virtual ~Rng() = default;

		virtual uint64_t GenerateNextNumber64() = 0;
		virtual Fixed GenerateNextNumberFixed() = 0;
	};
}
