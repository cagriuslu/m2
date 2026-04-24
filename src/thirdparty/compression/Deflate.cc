#include <m2/thirdparty/compression/Deflate.h>
#include <zlib.h>

using namespace m2;
using namespace m2::thirdparty;

expected<std::vector<uint8_t>> compression::Deflate(const std::vector<uint8_t>& source) {
	const auto bound = compressBound(source.size());
	std::vector<uint8_t> destination(bound, 0);
	auto destLen = bound;
	if (const auto result = compress(destination.data(), &destLen, source.data(), source.size()); result == Z_OK) {
		// Shrink destination to actual size after compression
		destination.resize(destLen);
		return std::move(destination);
	} else {
		const auto* errorMsg = zError(result);
		return make_unexpected(std::format("zlib error: sourceSize={}, compressBound={}, errorCode={}, errorMsg={}", source.size(), bound, result, errorMsg ? errorMsg : "<null>"));
	}
}
