#include <m2/third_party/physics/ColliderCategory.h>

using namespace m2::third_party::physics;

namespace {
	constexpr uint16_t gBackgroundLayers = COLLIDER_LAYER_BACKGROUND_OBSTACLE
			| COLLIDER_LAYER_BACKGROUND_FRIENDLY_OBJECT
			| COLLIDER_LAYER_BACKGROUND_FRIENDLY_DAMAGE
			| COLLIDER_LAYER_BACKGROUND_HOSTILE_OBJECT
			| COLLIDER_LAYER_BACKGROUND_HOSTILE_DAMAGE;
}

bool m2::third_party::physics::DoesBelongToBackground(uint16_t belongsTo) {
	return belongsTo & gBackgroundLayers;
}
