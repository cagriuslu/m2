#include <m2/thirdparty/compression/Inflate.h>
#include <zlib.h>

using namespace m2;
using namespace m2::thirdparty;

expected<std::vector<uint8_t>> compression::Inflate(const std::vector<uint8_t>& compressed, const int uncompressedSize) {
	std::vector<uint8_t> destination(uncompressedSize, 0);
	unsigned long destLen = uncompressedSize;
	if (const auto result = uncompress(destination.data(), &destLen, compressed.data(), compressed.size()); result == Z_OK) {
		return std::move(destination);
	} else {
		const auto* errorMsg = zError(result);
		return make_unexpected(std::format("zlib uncompress error: compressedSize={} uncompressSize={} errorCode={} errorMsg={}", compressed.size(), uncompressedSize, result, errorMsg ? errorMsg : "<null>"));
	}
}
