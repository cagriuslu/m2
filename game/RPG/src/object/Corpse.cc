#include <rpg/object/Corpse.h>
#include <m2/Game.h>

m2::void_expected rpg::create_corpse(m2::Object& obj, m2g::pb::SpriteType st) {
	auto& gfx = obj.add_terrain_graphic(GAME.get_sprite(st));
	gfx.draw_sprite_effect = m2::pb::SPRITE_EFFECT_IMAGE_ADJUSTMENT;

	return {};
}
