#include <m2/math/VecF.h>
#include <m2/Object.h>
#include "m2/Game.h"
#include <rpg/Objects.h>
#include <m2/box2d/Detail.h>
#include <m2/Log.h>

using namespace m2g;
using namespace m2g::pb;

m2::void_expected rpg::create_blade(m2::Object &obj, const m2::VecF &direction, const m2::Item &melee_weapon, bool is_friend) {
	// Check if weapon has necessary attributes
	if (!melee_weapon.has_attribute(ATTRIBUTE_AVERAGE_DAMAGE)) {
		throw M2_ERROR("Melee weapon has no average damage");
	}
	if (!melee_weapon.has_attribute(ATTRIBUTE_AVERAGE_TTL)) {
		throw M2_ERROR("Melee weapon has no average TTL");
	}
	float average_damage = melee_weapon.get_attribute(ATTRIBUTE_AVERAGE_DAMAGE);
	float damage_accuracy = melee_weapon.try_get_attribute(ATTRIBUTE_DAMAGE_ACCURACY, 1.0f);
	float average_ttl = melee_weapon.get_attribute(ATTRIBUTE_AVERAGE_TTL);

	const float direction_angle = direction.angle_rads();
	constexpr float swing_angle = m2::to_radians(120.0f); // Swing angle is 120 degrees
	const float start_angle = direction_angle + swing_angle / 2.0f;
	const float swing_speed = swing_angle / average_ttl;

	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(melee_weapon.game_sprite()));

	// Add physics
	auto& phy = obj.add_physique();
	auto bp = m2::box2d::example_bullet_body_blueprint();
	bp.mutable_foreground_fixture()->mutable_rect()->mutable_dims()->set_w(sprite.ForegroundColliderRectDimsM().x);
	bp.mutable_foreground_fixture()->mutable_rect()->mutable_dims()->set_h(sprite.ForegroundColliderRectDimsM().y);
	bp.mutable_foreground_fixture()->mutable_rect()->mutable_center_offset()->set_x(sprite.ForegroundColliderOriginToOriginVecM().x);
	bp.mutable_foreground_fixture()->mutable_rect()->mutable_center_offset()->set_y(sprite.ForegroundColliderOriginToOriginVecM().y);
	bp.mutable_foreground_fixture()->set_is_sensor(true);
	bp.mutable_foreground_fixture()->set_category(is_friend ? m2::pb::FixtureCategory::FRIEND_OFFENSE_ON_FOREGROUND : m2::pb::FixtureCategory::FOE_OFFENSE_ON_FOREGROUND);
	phy.body = m2::box2d::create_body(*M2_LEVEL.world, obj.physique_id(), obj.position, bp);
	phy.body->SetTransform(static_cast<b2Vec2>(obj.position), start_angle);
	phy.body->SetAngularVelocity(-swing_speed);

	// Add graphics
	auto& gfx = obj.add_graphic(melee_weapon.game_sprite());
	gfx.draw_angle = phy.body->GetAngle();
	gfx.z = 0.5f;

	// Add character
	auto& chr = obj.add_tiny_character();
	chr.add_named_item(M2_GAME.GetNamedItem(ITEM_AUTOMATIC_TTL));
	chr.add_resource(RESOURCE_TTL, average_ttl);

	chr.update = [](m2::Character& chr) {
		if (!chr.has_resource(RESOURCE_TTL)) {
			M2_DEFER(m2::create_object_deleter(chr.owner_id()));
		}
	};
	phy.on_collision = [average_damage, damage_accuracy](MAYBE m2::Physique& phy, m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		if (auto* other_char = other.owner().get_character(); other_char) {
			InteractionData data;
			data.set_hit_damage(m2::apply_accuracy(average_damage, average_damage, damage_accuracy));
			other_char->execute_interaction(data);
			// TODO knock-back
		}
	};
	phy.post_step = [&](m2::Physique& phy) {
		if (auto* originator = obj.get_parent()) {
			float curr_angle = phy.body->GetAngle();
			phy.body->SetTransform(static_cast<b2Vec2>(originator->position), curr_angle);
			obj.graphic().draw_angle = curr_angle;
		} else {
			// Originator died
			M2_DEFER(m2::create_object_deleter(phy.owner_id()));
		}
	};

	return {};
}
