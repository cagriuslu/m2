#include "m2/game/object/Tile.h"

#include "m2/Game.h"
#include "m2/Object.h"
#include "m2/box2d/Detail.h"

std::pair<m2::Object&, m2::Id> m2::obj::create_tile(BackgroundLayer layer, const VecF& position, const m2::Sprite& sprite) {
    auto obj_pair = create_object(position);
	obj_pair.first.add_terrain_graphic(layer, sprite);

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
			throw M2FATAL("Circular tile background_collider unimplemented");
		}

        // Use foreground collider as a secondary background collider
        if (sprite.foreground_collider_type() == box2d::ColliderType::RECTANGLE) {
            bp.mutable_foreground_fixture()->mutable_rect()->mutable_dims()->set_w(sprite.foreground_collider_rect_dims_m().x);
            bp.mutable_foreground_fixture()->mutable_rect()->mutable_dims()->set_h(sprite.foreground_collider_rect_dims_m().y);
            bp.mutable_foreground_fixture()->mutable_rect()->mutable_center_offset()->set_x(sprite.foreground_collider_origin_to_origin_vec_m().x);
            bp.mutable_foreground_fixture()->mutable_rect()->mutable_center_offset()->set_y(sprite.foreground_collider_origin_to_origin_vec_m().y);
            bp.mutable_foreground_fixture()->set_category(m2::pb::FixtureCategory::OBSTACLE_BACKGROUND);
        } else if (sprite.foreground_collider_type() == box2d::ColliderType::CIRCLE) {
            throw M2FATAL("Circular tile foreground_collider unimplemented");
        }

        auto& phy = obj_pair.first.add_physique();
        phy.body = m2::box2d::create_body(*LEVEL.world, obj_pair.first.physique_id(), obj_pair.first.position, bp);
	}

	if (sprite.has_foreground_companion()) {
		obj::create_tile_foreground_companion(position, sprite);
	}

    return obj_pair;
}

std::pair<m2::Object&, m2::Id> m2::obj::create_tile_foreground_companion(const VecF& position, const m2::Sprite& sprite) {
	auto obj_pair = create_object(position - sprite.center_offset_m() + sprite.foreground_companion_center_to_origin_vec_m());
	auto& companion = obj_pair.first;

	auto& gfx = companion.add_graphic(sprite);
	gfx.draw_variant = IsForegroundCompanion{true};

	return obj_pair;
}
