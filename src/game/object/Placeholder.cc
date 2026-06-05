#include "m2/game/object/Placeholder.h"
#include <m2/Game.h>

m2::Id m2::obj::CreateBackgroundPlaceholder(const VecF& pos, m2g::pb::SpriteType spriteType, m2g::pb::FlatGraphicsLayer layer) {
	const auto it = CreateObject();
	it->AddGraphic(layer, spriteType, pos);
	return it.GetId();
}

m2::Id m2::obj::CreateForegroundPlaceholder(const VecF& pos, float orientation, const std::optional<m2g::pb::SpriteType>& spriteType) {
	const auto it = CreateObject();
	if (spriteType) {
		it->AddGraphic(m2g::pb::UPRIGHT_GRAPHICS_DEFAULT_LAYER, *spriteType, pos).orientation = orientation;
	}
	return it.GetId();
}
