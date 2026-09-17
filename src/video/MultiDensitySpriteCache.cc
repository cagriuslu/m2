#include <m2/video/MultiDensitySpriteCache.h>
#include <m2/common/math/Hash.h>

namespace {
	/// Arbitrary non-zero seed
	constexpr int32_t DENSITY_HASH_SEED = 0x5E1F0D3B;
}

size_t m2::MultiDensitySpriteCache::DensityHash::operator()(const VecF& density) const {
	// HashI is bit-exact on floats, which is exactly what VecF::operator== compares, so equal keys always hash
	// equal. The detour through uint32_t stops a negative int32_t from sign-extending across the whole size_t.
	return static_cast<size_t>(static_cast<uint32_t>(
			HashI(density.GetY(), HashI(density.GetX(), DENSITY_HASH_SEED))));
}

m2::SpriteCache& m2::MultiDensitySpriteCache::CreateOrGet(const VecF& density) {
	if (not std::isfinite(density.GetX()) || not std::isfinite(density.GetY())) {
		throw M2_ERROR("Window pixel density is not finite");
	}
	if (density.GetX() <= 0.0f || density.GetY() <= 0.0f) {
		throw M2_ERROR("Window pixel density is not positive");
	}
	return _cache(density);
}
