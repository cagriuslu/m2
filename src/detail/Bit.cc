#include <m2/detail/Bit.h>
#include <m2/Error.h>
#include <m2/Meta.h>

uint64_t m2::RotateLeft64(uint64_t n, const uint8_t shiftCount) {
	return (n << shiftCount) | (n >> (64 - shiftCount));
}
uint64_t m2::RotateRight64(uint64_t n, const uint8_t shiftCount) {
	return (n >> shiftCount) | (n << (64 - shiftCount));
}

uint64_t m2::PickSequentialBits(uint64_t n, const uint8_t leastSignificantBit, const uint8_t numberOfBits) {
	if (63 < leastSignificantBit) {
		throw M2_ERROR("Invalid least significant bit");
	}
	if (64 < numberOfBits) {
		throw M2_ERROR("Invalid number of bits");
	}

	// Rotate the number right until the least significant bit is at position 0
	n = RotateRight64(n, leastSignificantBit);
	// Prepare the mask to clear the bits to the left of position (numberOfBits - 1)
	uint64_t mask{};
	m2Repeat(numberOfBits) {
		mask = (mask << 1) | 0x01;
	}
	// Mask
	return n & mask;
}
