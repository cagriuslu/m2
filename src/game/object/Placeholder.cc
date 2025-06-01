#include "m2/game/object/Placeholder.h"

m2::Id m2::obj::create_background_placeholder(const VecF& pos, m2g::pb::SpriteType spriteType, BackgroundDrawLayer layer) {
	const auto it = CreateObject(pos);
	it->AddGraphic(layer, spriteType);
	return it.GetId();
}

m2::Id m2::obj::create_foreground_placeholder(const VecF& pos, float orientation, const std::optional<m2g::pb::SpriteType>& spriteType) {
	const auto it = CreateObject(pos);
	it->orientation = orientation;
	if (spriteType) {
		it->AddGraphic(ForegroundDrawLayer::F0_BOTTOM, *spriteType);
	}
	return it.GetId();
}
