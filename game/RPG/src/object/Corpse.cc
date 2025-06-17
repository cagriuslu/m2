#include <rpg/Objects.h>
#include <m2/Game.h>

m2::void_expected rpg::create_corpse(m2::Object& obj, m2g::pb::SpriteType st) {
	obj.AddGraphic(m2::BackgroundDrawLayer::B0, st);
	return {};
}
