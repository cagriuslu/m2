#include <m2g/SpriteBlueprint.h>
#include <m2/SDLUtils.hh>
#include <m2/object/Tile.h>
#include <m2/Object.h>
#include "m2/Game.hh"
#include <m2/box2d/Utils.h>

std::pair<m2::Object&, m2::ID> m2::obj::create_tile(const Vec2f& position, const m2::Sprite& sprite) {
    auto obj_pair = create_object(position);
	auto& tile = obj_pair.first;

	if (sprite.sprite().has_collider()) {
		auto& collider = sprite.sprite().collider();
		if (collider.has_rect_dims_px()) {
			auto& phy = tile.add_physique();
			phy.body = m2::box2d::create_static_box(*GAME.world, tile.physique_id(), position, false, m2::box2d::CAT_GND_OBSTACLE,  Vec2f{collider.rect_dims_px()} / sprite.ppm());
		} else {
			throw M2FATAL("Circular tile collider unimplemented");
		}
	}

    auto& gfx = tile.add_terrain_graphic(sprite);

    return obj_pair;
}
