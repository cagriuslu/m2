#pragma once
#include <m2/Meta.h>
#include <vector>

namespace m2::thirdparty::compression {
	expected<std::vector<uint8_t>> Inflate(const std::vector<uint8_t>& compressed, int uncompressedSize);
}
