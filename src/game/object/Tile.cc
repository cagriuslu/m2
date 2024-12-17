#include "m2/game/object/Tile.h"

#include "m2/Game.h"
#include "m2/Object.h"
#include "m2/box2d/Detail.h"

m2::Pool<m2::Object>::Iterator m2::obj::create_tile(BackgroundLayer layer, const VecF& position, const m2::Sprite& sprite) {
    auto it = create_object(position);
	it->add_terrain_graphic(layer, sprite);

	if (sprite.BackgroundColliderType() != box2d::ColliderType::NONE) {
        m2::pb::BodyBlueprint bp;

		if (sprite.BackgroundColliderType() == box2d::ColliderType::RECTANGLE) {
			bp.set_type(m2::pb::BodyType::STATIC);
			bp.set_allow_sleep(true);
			bp.mutable_background_fixture()->mutable_rect()->mutable_dims()->set_w(sprite.BackgroundColliderRectDimsM().x);
			bp.mutable_background_fixture()->mutable_rect()->mutable_dims()->set_h(sprite.BackgroundColliderRectDimsM().y);
            bp.mutable_background_fixture()->mutable_rect()->mutable_center_offset()->set_x(
			    sprite.BackgroundColliderOriginToOriginVecM().x);
            bp.mutable_background_fixture()->mutable_rect()->mutable_center_offset()->set_y(
			    sprite.BackgroundColliderOriginToOriginVecM().y);
			bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::OBSTACLE_BACKGROUND);
		} else if (sprite.BackgroundColliderType() == box2d::ColliderType::CIRCLE) {
			throw M2_ERROR("Circular tile background_collider unimplemented");
		}

        // Use foreground collider as a secondary background collider
        if (sprite.ForegroundColliderType() == box2d::ColliderType::RECTANGLE) {
            bp.mutable_foreground_fixture()->mutable_rect()->mutable_dims()->set_w(sprite.ForegroundColliderRectDimsM().x);
            bp.mutable_foreground_fixture()->mutable_rect()->mutable_dims()->set_h(sprite.ForegroundColliderRectDimsM().y);
            bp.mutable_foreground_fixture()->mutable_rect()->mutable_center_offset()->set_x(sprite.ForegroundColliderOriginToOriginVecM().x);
            bp.mutable_foreground_fixture()->mutable_rect()->mutable_center_offset()->set_y(sprite.ForegroundColliderOriginToOriginVecM().y);
            bp.mutable_foreground_fixture()->set_category(m2::pb::FixtureCategory::OBSTACLE_BACKGROUND);
        } else if (sprite.ForegroundColliderType() == box2d::ColliderType::CIRCLE) {
            throw M2_ERROR("Circular tile foreground_collider unimplemented");
        }

        auto& phy = it->add_physique();
        phy.body = m2::box2d::create_body(*M2_LEVEL.world, it->physique_id(), it->position, bp);
	}

	if (sprite.HasForegroundCompanion()) {
		obj::create_tile_foreground_companion(position, sprite);
	}

    return it;
}

m2::Pool<m2::Object>::Iterator m2::obj::create_tile_foreground_companion(const VecF& position, const m2::Sprite& sprite) {
	auto it = create_object(position - sprite.CenterToOriginVecM() + sprite.ForegroundCompanionCenterToOriginVecM());

	auto& gfx = it->add_graphic(sprite);
	gfx.variant_draw_order[0] = ForegroundCompanion{};

	return it;
}
