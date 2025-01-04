#include <rpg/Objects.h>
#include <m2/Game.h>

m2::void_expected rpg::create_decoration(m2::Object& obj, m2g::pb::SpriteType sprite_type) {
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(sprite_type));
	auto& gfx = obj.add_graphic(sprite_type);
	if (obj.object_type() == m2g::pb::FENCE_VERTICAL) {
		gfx.draw_angle = m2::PI_DIV2;
	}

	if (sprite.BackgroundColliderType() == m2::box2d::ColliderType::RECTANGLE) {
		m2::pb::BodyBlueprint bp;
		bp.set_type(m2::pb::BodyType::STATIC);
		bp.set_allow_sleep(true);
		bp.mutable_background_fixture()->mutable_rect()->mutable_center_offset()->set_x(
		    sprite.BackgroundColliderOriginToOriginVecM().x);
		bp.mutable_background_fixture()->mutable_rect()->mutable_center_offset()->set_y(
		    sprite.BackgroundColliderOriginToOriginVecM().y);
		bp.mutable_background_fixture()->mutable_rect()->mutable_dims()->set_w(sprite.BackgroundColliderRectDimsM().x);
		bp.mutable_background_fixture()->mutable_rect()->mutable_dims()->set_h(sprite.BackgroundColliderRectDimsM().y);
		if (obj.object_type() == m2g::pb::FENCE_VERTICAL) {
			bp.mutable_background_fixture()->mutable_rect()->set_angle(m2::PI_DIV2);
		}
		bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::OBSTACLE_BACKGROUND);

		auto& phy = obj.add_physique();
		phy.body = m2::box2d::create_body(*M2_LEVEL.world, obj.physique_id(), obj.position, bp);
	} else if (sprite.BackgroundColliderType() == m2::box2d::ColliderType::CIRCLE) {
		m2::pb::BodyBlueprint bp;
		bp.set_type(m2::pb::BodyType::STATIC);
		bp.set_allow_sleep(true);
		bp.mutable_background_fixture()->mutable_circ()->mutable_center_offset()->set_x(
		    sprite.BackgroundColliderOriginToOriginVecM().x);
		bp.mutable_background_fixture()->mutable_circ()->mutable_center_offset()->set_y(
		    sprite.BackgroundColliderOriginToOriginVecM().y);
		bp.mutable_background_fixture()->mutable_circ()->set_radius(sprite.BackgroundColliderCircRadiusM());
		bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::OBSTACLE_BACKGROUND);

		auto& phy = obj.add_physique();
		phy.body = m2::box2d::create_body(*M2_LEVEL.world, obj.physique_id(), obj.position, bp);
	}

	return {};
}
