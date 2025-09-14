#include "m2/game/object/Placeholder.h"

m2::Id m2::obj::CreateBackgroundPlaceholder(const VecF& pos, m2g::pb::SpriteType spriteType, pb::FlatGraphicsLayer layer) {
	const auto it = CreateObject();
	it->AddGraphic(layer, spriteType, pos);
	return it.GetId();
}

m2::Id m2::obj::CreateForegroundPlaceholder(const VecF& pos, float orientation, const std::optional<m2g::pb::SpriteType>& spriteType) {
	const auto it = CreateObject();
	if (spriteType) {
		it->AddGraphic(pb::UprightGraphicsLayer::SEA_LEVEL_UPRIGHT, *spriteType, pos).orientation = orientation;
	}
	return it.GetId();
}
