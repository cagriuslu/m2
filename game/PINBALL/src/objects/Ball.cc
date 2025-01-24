#include <pinball/objects/Ball.h>
#include <m2/Game.h>

m2::void_expected LoadBall(m2::Object& obj) {
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(m2g::pb::SPRITE_BASIC_BALL));

	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::DYNAMIC);
	bp.mutable_foreground_fixture()->mutable_circ()->set_radius(sprite.ForegroundColliderCircRadiusM());
	bp.mutable_foreground_fixture()->set_category(m2::pb::FixtureCategory::FOE_ON_FOREGROUND);
	bp.set_mass(10.0f);
	bp.set_gravity_scale(4.0f);
	bp.set_linear_damping(0.0f);
	bp.set_fixed_rotation(true);
	bp.set_is_bullet(true);
	phy.body = m2::box2d::create_body(*M2_LEVEL.world, obj.physique_id(), obj.position, bp);
	phy.post_step = [](m2::Physique& phy_) {};

	MAYBE auto& gfx = obj.add_graphic(m2g::pb::SPRITE_BASIC_BALL);

	return {};
}
