#include <m2/VecF.h>
#include <m2/Object.h>
#include "m2/Game.h"
#include <rpg/object/Blade.h>
#include <m2/box2d/Detail.h>
#include <m2/Log.h>

using namespace m2g;
using namespace m2g::pb;

m2::VoidValue rpg::create_blade(m2::Object &obj, const m2::VecF &direction, const m2::Item &melee_weapon, bool is_friend) {
	// Check if weapon has necessary attributes
	if (!melee_weapon.has_attribute(ATTRIBUTE_AVERAGE_DAMAGE)) {
		throw M2ERROR("Melee weapon has no average damage");
	}
	if (!melee_weapon.has_attribute(ATTRIBUTE_AVERAGE_TTL)) {
		throw M2ERROR("Melee weapon has no average TTL");
	}
	float average_damage = melee_weapon.get_attribute(ATTRIBUTE_AVERAGE_DAMAGE);
	float damage_accuracy = melee_weapon.try_get_attribute(ATTRIBUTE_DAMAGE_ACCURACY, 1.0f);
	float average_ttl = melee_weapon.get_attribute(ATTRIBUTE_AVERAGE_TTL);

	const float direction_angle = direction.angle_rads();
	constexpr float swing_angle = m2::to_radians(120.0f); // Swing angle is 120 degrees
	const float start_angle = direction_angle + swing_angle / 2.0f;
	const float swing_speed = swing_angle / average_ttl;

	const auto& sprite = GAME.get_sprite(melee_weapon.game_sprite());

	// Add physics
	auto& phy = obj.add_physique();
	auto bp = m2::box2d::example_bullet_body_blueprint();
	bp.mutable_foreground_fixture()->mutable_rect()->mutable_dims()->set_w(sprite.foreground_collider_rect_dims_m().x);
	bp.mutable_foreground_fixture()->mutable_rect()->mutable_dims()->set_h(sprite.foreground_collider_rect_dims_m().y);
	bp.mutable_foreground_fixture()->mutable_rect()->mutable_center_offset()->set_x(sprite.foreground_collider_center_offset_m().x);
	bp.mutable_foreground_fixture()->mutable_rect()->mutable_center_offset()->set_y(sprite.foreground_collider_center_offset_m().y);
	bp.mutable_foreground_fixture()->set_is_sensor(true);
	bp.mutable_foreground_fixture()->set_category(is_friend ? m2::pb::FixtureCategory::FRIEND_OFFENSE_ON_FOREGROUND : m2::pb::FixtureCategory::FOE_OFFENSE_ON_FOREGROUND);
	phy.body = m2::box2d::create_body(*LEVEL.world, obj.physique_id(), obj.position, bp);
	phy.body->SetTransform(static_cast<b2Vec2>(obj.position), start_angle);
	phy.body->SetAngularVelocity(-swing_speed);

	// Add graphics
	auto& gfx = obj.add_graphic(sprite);
	gfx.draw_angle = phy.body->GetAngle();
	gfx.z = 0.5f;

	// Add character
	auto& chr = obj.add_tiny_character();
	chr.add_item(GAME.get_item(ITEM_AUTOMATIC_TTL));
	chr.add_resource(RESOURCE_TTL, average_ttl);

	chr.update = [](m2::Character& chr) {
		if (!chr.has_resource(RESOURCE_TTL)) {
			GAME.add_deferred_action(m2::create_object_deleter(chr.object_id));
		}
	};
	phy.on_collision = [&](MAYBE m2::Physique& phy, m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		auto& other_obj = other.parent();
		if (other_obj.character_id()) {
			m2::Character::execute_interaction(chr, other_obj.character(), InteractionType::HIT);
			// TODO knock-back
		}
	};
	chr.create_interaction = [=](MAYBE m2::Character& self, m2::Character& other, InteractionType interaction_type) -> std::optional<InteractionData> {
		if (interaction_type == InteractionType::HIT) {
			// Calculate damage
			InteractionData data;
			data.set_hit_damage(m2::apply_accuracy(average_damage, average_damage, damage_accuracy));
			return data;
		}
		return std::nullopt;
	};
	phy.post_step = [&](m2::Physique& phy) {
		auto* originator = obj.parent();
		if (originator) {
			float curr_angle = phy.body->GetAngle();
			phy.body->SetTransform(static_cast<b2Vec2>(originator->position), curr_angle);
			obj.graphic().draw_angle = curr_angle;
		} else {
			// Originator died
			GAME.add_deferred_action(m2::create_object_deleter(phy.object_id));
		}
	};

	return {};
}
