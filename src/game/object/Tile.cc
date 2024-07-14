#include "m2/game/object/Tile.h"

#include "m2/Game.h"
#include "m2/Object.h"
#include "m2/box2d/Detail.h"

m2::Pool<m2::Object>::Iterator m2::obj::create_tile(BackgroundLayer layer, const VecF& position, const m2::Sprite& sprite) {
    auto it = create_object(position);
	it->add_terrain_graphic(layer, sprite);

	if (sprite.background_collider_type() != box2d::ColliderType::NONE) {
        m2::pb::BodyBlueprint bp;

		if (sprite.background_collider_type() == box2d::ColliderType::RECTANGLE) {
			bp.set_type(m2::pb::BodyType::STATIC);
			bp.set_allow_sleep(true);
			bp.mutable_background_fixture()->mutable_rect()->mutable_dims()->set_w(sprite.background_collider_rect_dims_m().x);
			bp.mutable_background_fixture()->mutable_rect()->mutable_dims()->set_h(sprite.background_collider_rect_dims_m().y);
            bp.mutable_background_fixture()->mutable_rect()->mutable_center_offset()->set_x(
			    sprite.background_collider_origin_to_origin_vec_m().x);
            bp.mutable_background_fixture()->mutable_rect()->mutable_center_offset()->set_y(
			    sprite.background_collider_origin_to_origin_vec_m().y);
			bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::OBSTACLE_BACKGROUND);
		} else if (sprite.background_collider_type() == box2d::ColliderType::CIRCLE) {
			throw M2_ERROR("Circular tile background_collider unimplemented");
		}

        // Use foreground collider as a secondary background collider
        if (sprite.foreground_collider_type() == box2d::ColliderType::RECTANGLE) {
            bp.mutable_foreground_fixture()->mutable_rect()->mutable_dims()->set_w(sprite.foreground_collider_rect_dims_m().x);
            bp.mutable_foreground_fixture()->mutable_rect()->mutable_dims()->set_h(sprite.foreground_collider_rect_dims_m().y);
            bp.mutable_foreground_fixture()->mutable_rect()->mutable_center_offset()->set_x(sprite.foreground_collider_origin_to_origin_vec_m().x);
            bp.mutable_foreground_fixture()->mutable_rect()->mutable_center_offset()->set_y(sprite.foreground_collider_origin_to_origin_vec_m().y);
            bp.mutable_foreground_fixture()->set_category(m2::pb::FixtureCategory::OBSTACLE_BACKGROUND);
        } else if (sprite.foreground_collider_type() == box2d::ColliderType::CIRCLE) {
            throw M2_ERROR("Circular tile foreground_collider unimplemented");
        }

        auto& phy = it->add_physique();
        phy.body = m2::box2d::create_body(*M2_LEVEL.world, it->physique_id(), it->position, bp);
	}

	if (sprite.has_foreground_companion()) {
		obj::create_tile_foreground_companion(position, sprite);
	}

    return it;
}

m2::Pool<m2::Object>::Iterator m2::obj::create_tile_foreground_companion(const VecF& position, const m2::Sprite& sprite) {
	auto it = create_object(position - sprite.center_to_origin_vec_m() + sprite.foreground_companion_center_to_origin_vec_m());

	auto& gfx = it->add_graphic(sprite);
	gfx.draw_variant = IsForegroundCompanion{true};

	return it;
}
