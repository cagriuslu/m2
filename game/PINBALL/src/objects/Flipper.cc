#include <pinball/objects/Flipper.h>
#include <m2/Game.h>

m2::void_expected LoadFlipper(m2::Object& obj, const bool rightFlipper) {
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(m2g::pb::SPRITE_BASIC_FLIPPER));

	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bb;
	bb.set_type(m2::pb::DYNAMIC);
	bb.set_allow_sleep(false);
	bb.mutable_foreground_fixture()->mutable_rect()->mutable_dims()->set_w(sprite.ForegroundColliderRectDimsM().x);
	bb.mutable_foreground_fixture()->mutable_rect()->mutable_dims()->set_h(sprite.ForegroundColliderRectDimsM().y);
	bb.mutable_foreground_fixture()->mutable_rect()->mutable_center_offset()->set_x(sprite.ForegroundColliderOriginToOriginVecM().x);
	bb.mutable_foreground_fixture()->mutable_rect()->mutable_center_offset()->set_y(sprite.ForegroundColliderOriginToOriginVecM().y);
	bb.mutable_foreground_fixture()->set_category(m2::pb::FRIEND_ON_FOREGROUND);
	bb.set_mass(100.0f);
	bb.set_gravity_scale(-1.0f);
	bb.set_fixed_rotation(false);
	bb.set_inertia(20.0f);
	phy.body = m2::box2d::create_body(*M2_LEVEL.world, obj.physique_id(), obj.position, bb);

	auto& gfx = obj.add_graphic(m2g::pb::SPRITE_BASIC_FLIPPER);

	if (not rightFlipper) {
		phy.pre_step = [](m2::Physique& phy_) {
			if (M2_GAME.events.pop_key_press(m2::Key::LEFT)) {
				phy_.body->ApplyTorque(-1000.0f, true);
			}
		};
		phy.post_step = [&gfx](m2::Physique& phy_) {
			gfx.draw_angle = phy_.body->GetAngle();
		};
	}

	return {};
}
