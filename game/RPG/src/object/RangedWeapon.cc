#include <m2/Object.h>
#include "m2/Game.h"
#include <rpg/object/RangedWeapon.h>
#include <m2/box2d/Detail.h>
#include <m2/M2.h>
#include <m2/Log.h>

using namespace m2g;
using namespace m2g::pb;

// TODO add other types of Ranged Weapons: Machine Gun, Shotgun, Bow

m2::VoidValue rpg::create_ranged_weapon_object(m2::Object& obj, const m2::Vec2f& intended_direction, const m2::Item& ranged_weapon) {
	// Check if weapon has necessary attributes
	if (!ranged_weapon.has_attribute(ATTRIBUTE_LINEAR_SPEED)) {
		throw M2ERROR("Ranged weapon has no linear speed");
	}
	if (!ranged_weapon.has_attribute(ATTRIBUTE_AVERAGE_DAMAGE)) {
		throw M2ERROR("Ranged weapon has no average damage");
	}
	if (!ranged_weapon.has_attribute(ATTRIBUTE_AVERAGE_TTL)) {
		throw M2ERROR("Ranged weapon has no average TTL");
	}

	float linear_speed = ranged_weapon.get_attribute(ATTRIBUTE_LINEAR_SPEED);
	float angular_accuracy = ranged_weapon.try_get_attribute(ATTRIBUTE_ANGULAR_ACCURACY, 1.0f);
	float average_damage = ranged_weapon.get_attribute(ATTRIBUTE_AVERAGE_DAMAGE);
	float damage_accuracy = ranged_weapon.try_get_attribute(ATTRIBUTE_DAMAGE_ACCURACY, 1.0f);
	float average_ttl = ranged_weapon.get_attribute(ATTRIBUTE_AVERAGE_TTL);
	float ttl_accuracy = ranged_weapon.try_get_attribute(ATTRIBUTE_TTL_ACCURACY, 1.0f);

	float angle = m2::apply_accuracy(intended_direction.angle_rads(), angular_accuracy);
	auto direction = m2::Vec2f::from_angle(angle);
	float ttl = m2::apply_accuracy(average_ttl, ttl_accuracy);

	// Add physics
	auto& phy = obj.add_physique();
	auto bp = m2::box2d::example_bullet_body_blueprint();
	bp.mutable_foreground_fixture()->mutable_circ()->set_radius(0.167f);
	bp.mutable_foreground_fixture()->set_is_sensor(true);
	bp.mutable_foreground_fixture()->set_category(m2::pb::FixtureCategory::FRIEND_OFFENSE_ON_FOREGROUND);
	bp.set_fixed_rotation(true);
	phy.body = m2::box2d::create_body(*LEVEL.world, obj.physique_id(), obj.position, bp);
	phy.body->SetLinearVelocity(static_cast<b2Vec2>(direction * linear_speed));

	// Add graphics
	auto& gfx = obj.add_graphic(GAME.get_sprite(SpriteType::BULLET_00));
	gfx.draw_angle = angle;

	// Add character
	auto& chr = obj.add_tiny_character();
	chr.add_item(GAME.get_item(ITEM_AUTOMATIC_TTL));
	chr.add_resource(RESOURCE_TTL, ttl);

	chr.update = [&](m2::Character& chr) {
		if (!chr.has_resource(RESOURCE_TTL)) {
			GAME.add_deferred_action(m2::create_object_deleter(chr.object_id));
		}
	};
	phy.on_collision = [&](MAYBE m2::Physique& phy, m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		auto& other_obj = other.parent();
		if (other_obj.character_id()) {
			m2::Character::execute_interaction(chr, InteractionType::COLLIDE_TO, other_obj.character(), InteractionType::GET_COLLIDED_BY);
			// TODO knock-back
		}
	};
	chr.interact = [=](m2::Character& self, m2::Character& other, InteractionType interaction_type) {
		if (interaction_type == InteractionType::COLLIDE_TO && self.has_resource(RESOURCE_TTL)) {
			// Calculate damage
			float damage = m2::apply_accuracy(average_damage, damage_accuracy);
			// Create and give damage item
			other.add_item(m2::make_damage_item(RESOURCE_HP, damage));
			// Clear TTL
			self.clear_resource(RESOURCE_TTL);
		}
	};

	return {};
}
