#pragma once
#include <m2/common/Meta.h>
#include <vector>

namespace m2::thirdparty::compression {
	expected<std::vector<uint8_t>> Deflate(const std::vector<uint8_t>& uncompressed);
}
