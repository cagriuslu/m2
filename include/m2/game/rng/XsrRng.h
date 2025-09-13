#pragma once
#include "Rng.h"
#include <array>

namespace m2 {
	class XsrRng : public Rng {
		std::array<uint64_t, 4> _xoshiro256ppState;

	public:
		XsrRng(uint64_t seed1, uint64_t seed2, uint64_t seed3, uint64_t seed4);
		~XsrRng() override = default;

		uint64_t GenerateNextNumber64() override;
		Exact GenerateNextNumberExact() override;
	};
}
