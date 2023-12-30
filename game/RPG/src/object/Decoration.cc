#include <rpg/Objects.h>
#include <m2/Game.h>

m2::void_expected rpg::create_decoration(m2::Object& obj, m2g::pb::SpriteType sprite_type) {
	const auto& sprite = GAME.get_sprite(sprite_type);

	auto& gfx = obj.add_graphic(sprite);
	if (obj.object_type() == m2g::pb::FENCE_VERTICAL) {
		gfx.draw_angle = m2::PI_DIV2;
	}

	if (sprite.background_collider_type() == m2::box2d::ColliderType::RECTANGLE) {
		m2::pb::BodyBlueprint bp;
		bp.set_type(m2::pb::BodyType::STATIC);
		bp.set_allow_sleep(true);
		bp.mutable_background_fixture()->mutable_rect()->mutable_center_offset()->set_x(sprite.background_collider_center_offset_m().x);
		bp.mutable_background_fixture()->mutable_rect()->mutable_center_offset()->set_y(sprite.background_collider_center_offset_m().y);
		bp.mutable_background_fixture()->mutable_rect()->mutable_dims()->set_w(sprite.background_collider_rect_dims_m().x);
		bp.mutable_background_fixture()->mutable_rect()->mutable_dims()->set_h(sprite.background_collider_rect_dims_m().y);
		if (obj.object_type() == m2g::pb::FENCE_VERTICAL) {
			bp.mutable_background_fixture()->mutable_rect()->set_angle(m2::PI_DIV2);
		}
		bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::OBSTACLE_BACKGROUND);

		auto& phy = obj.add_physique();
		phy.body = m2::box2d::create_body(*LEVEL.world, obj.physique_id(), obj.position, bp);
	} else if (sprite.background_collider_type() == m2::box2d::ColliderType::CIRCLE) {
		m2::pb::BodyBlueprint bp;
		bp.set_type(m2::pb::BodyType::STATIC);
		bp.set_allow_sleep(true);
		bp.mutable_background_fixture()->mutable_circ()->mutable_center_offset()->set_x(sprite.background_collider_center_offset_m().x);
		bp.mutable_background_fixture()->mutable_circ()->mutable_center_offset()->set_y(sprite.background_collider_center_offset_m().y);
		bp.mutable_background_fixture()->mutable_circ()->set_radius(sprite.background_collider_circ_radius_m());
		bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::OBSTACLE_BACKGROUND);

		auto& phy = obj.add_physique();
		phy.body = m2::box2d::create_body(*LEVEL.world, obj.physique_id(), obj.position, bp);
	}

	return {};
}
