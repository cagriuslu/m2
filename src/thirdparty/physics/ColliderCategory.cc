#include <m2/thirdparty/physics/ColliderCategory.h>

using namespace m2::thirdparty::physics;

namespace {
	constexpr uint16_t gBackgroundLayers = COLLIDER_LAYER_BACKGROUND_OBSTACLE
			| COLLIDER_LAYER_BACKGROUND_FRIENDLY_OBJECT
			| COLLIDER_LAYER_BACKGROUND_FRIENDLY_DAMAGE
			| COLLIDER_LAYER_BACKGROUND_HOSTILE_OBJECT
			| COLLIDER_LAYER_BACKGROUND_HOSTILE_DAMAGE;
}

bool m2::thirdparty::physics::DoesBelongToBackground(uint16_t belongsTo) {
	return belongsTo & gBackgroundLayers;
}
