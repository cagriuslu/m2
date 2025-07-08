#pragma once
#include <cstdint>

namespace m2 {
	uint64_t RotateLeft64(uint64_t n, uint8_t shiftCount);
	uint64_t RotateRight64(uint64_t n, uint8_t shiftCount);

	uint64_t PickSequentialBits(uint64_t n, uint8_t leastSignificantBit, uint8_t numberOfBits);
}
