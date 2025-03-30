#include "m2/game/object/Placeholder.h"

m2::Id m2::obj::create_background_placeholder(const VecF& pos, m2g::pb::SpriteType spriteType, BackgroundLayer layer) {
	auto it = CreateObject(pos);
	it->AddTerrainGraphic(layer, spriteType);
	return it.GetId();
}

m2::Id m2::obj::create_foreground_placeholder(const VecF& pos, float orientation, m2g::pb::SpriteType spriteType) {
	auto it = CreateObject(pos);
	it->orientation = orientation;
	it->AddGraphic(spriteType);
	return it.GetId();
}
