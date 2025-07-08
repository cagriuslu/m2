#pragma once
#include <cstdint>

namespace m2 {
	class Rng {
	public:
		virtual ~Rng() = default;

		virtual uint64_t GenerateNextNumber() = 0;
	};
}
