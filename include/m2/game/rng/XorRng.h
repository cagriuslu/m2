#pragma once
#include "Rng.h"

namespace m2 {
	class XorRng : public Rng {
		uint64_t _prevNumber;

	public:
		explicit XorRng(uint64_t seed1);
		~XorRng() override = default;

		uint64_t GenerateNextNumber() override;
	};
}
