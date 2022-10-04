#include <m2g/Object.h>
#include <m2/SdlUtils.hh>
#include <m2/object/Tile.h>
#include <m2/Object.h>
#include "m2/Game.hh"
#include <m2/box2d/Utils.h>

std::pair<m2::Object&, m2::Id> m2::obj::create_tile(const Vec2f& position, const m2::Sprite& sprite) {
    auto obj_pair = create_object(position);
	auto& tile = obj_pair.first;

	if (sprite.sprite().has_collider()) {
		auto& collider = sprite.sprite().collider();
		if (collider.has_rect_dims_px()) {
			auto& phy = tile.add_physique();
			m2::pb::BodyBlueprint bp;
			bp.set_type(m2::pb::BodyType::STATIC);
			bp.mutable_rect()->mutable_dims()->set_w(collider.rect_dims_px().w() / (float)sprite.ppm());
			bp.mutable_rect()->mutable_dims()->set_h(collider.rect_dims_px().h() / (float)sprite.ppm());
			bp.set_allow_sleep(true);
			bp.set_is_bullet(false);
			bp.set_is_sensor(false);
			bp.set_category(m2::pb::BodyCategory::OBSTACLE_BACKGROUND);
			phy.body = m2::box2d::create_body(*GAME.world, tile.physique_id(), position, bp);
		} else {
			throw M2FATAL("Circular tile collider unimplemented");
		}
	}

    auto& gfx = tile.add_terrain_graphic(sprite);

    return obj_pair;
}
