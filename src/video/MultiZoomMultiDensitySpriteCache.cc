#include <m2/video/MultiZoomMultiDensitySpriteCache.h>
#include <m2/common/Error.h>
#include <m2/common/Meta.h>
#include <format>
#include <limits>

m2::MultiDensitySpriteCache& m2::MultiZoomMultiDensitySpriteCache::CreateOrGet(const float gameHeightM) {
	// The range is checked before rounding, because RoundI is undefined for NaN, for infinities, and for values that
	// don't fit in an int. NaN fails every comparison, so it is rejected too. 0.5 is the smallest value that rounds to 1.
	if (not (0.5f <= gameHeightM && gameHeightM < static_cast<float>(std::numeric_limits<int>::max()))) {
		throw M2_ERROR(std::format("Game height doesn't round to a positive integer: {}", gameHeightM));
	}
	return _cache(RoundI(gameHeightM));
}
