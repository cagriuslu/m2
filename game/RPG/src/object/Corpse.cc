#include <rpg/Objects.h>
#include <m2/Game.h>

m2::void_expected rpg::create_corpse(m2::Object& obj, m2g::pb::SpriteType st) {
	auto& gfx = obj.add_terrain_graphic(m2::BackgroundLayer::L0, st);
	gfx.variantDrawOrder[0] = m2::pb::SPRITE_EFFECT_IMAGE_ADJUSTMENT;

	return {};
}
