#include <rpg/object/ExplosiveWeapon.h>
#include <m2/Object.h>
#include "m2/Game.h"
#include <m2g/Object.h>
#include <m2/box2d/Utils.h>
#include <m2/M2.h>
#include <m2/Log.h>

using namespace m2g;
using namespace m2g::pb;

enum class ExplosiveState {
	IN_FLIGHT,
	WILL_EXPLODE,
	EXPLODING
};

struct ExplosiveObjectImpl : public m2::ObjectImpl {
	ExplosiveState explosive_state{ExplosiveState::IN_FLIGHT};
};

m2::VoidValue rpg::create_explosive_object(m2::Object& obj, const m2::Vec2f& intended_direction, const m2::Item& explosive_weapon) {
	// Check if weapon has necessary attributes
	if (!explosive_weapon.has_attribute(ATTRIBUTE_LINEAR_SPEED)) {
		throw M2ERROR("Explosive weapon has no linear speed");
	}
	if (!explosive_weapon.has_attribute(ATTRIBUTE_AVERAGE_DAMAGE)) {
		throw M2ERROR("Ranged weapon has no average damage");
	}
	if (!explosive_weapon.has_attribute(ATTRIBUTE_AVERAGE_TTL)) {
		throw M2ERROR("Ranged weapon has no average TTL");
	}

	float linear_speed = explosive_weapon.get_attribute(ATTRIBUTE_LINEAR_SPEED);
	float angular_accuracy = explosive_weapon.try_get_attribute(ATTRIBUTE_ANGULAR_ACCURACY, 1.0f);
	float average_damage = explosive_weapon.get_attribute(ATTRIBUTE_AVERAGE_DAMAGE);
	float damage_accuracy = explosive_weapon.try_get_attribute(ATTRIBUTE_DAMAGE_ACCURACY, 1.0f);
	float damage_radius = explosive_weapon.try_get_attribute(ATTRIBUTE_DAMAGE_RADIUS, 0.1f);
	float average_ttl = explosive_weapon.get_attribute(ATTRIBUTE_AVERAGE_TTL);
	float ttl_accuracy = explosive_weapon.try_get_attribute(ATTRIBUTE_TTL_ACCURACY, 1.0f);

	float angle = m2::apply_accuracy(intended_direction.angle_rads(), angular_accuracy);
	auto direction = m2::Vec2f::from_angle(angle);
	float ttl = m2::apply_accuracy(average_ttl, ttl_accuracy);

	// Add physics
	auto& phy = obj.add_physique();
	auto bp = m2::box2d::example_bullet_body_blueprint();
	bp.mutable_foreground_fixture()->mutable_circ()->set_radius(0.25f);
	bp.mutable_foreground_fixture()->set_is_sensor(true);
	bp.mutable_foreground_fixture()->set_category(m2::pb::FixtureCategory::FRIEND_OFFENSE_ON_FOREGROUND);
	bp.set_fixed_rotation(true);
	phy.body = m2::box2d::create_body(*LEVEL.world, obj.physique_id(), obj.position, bp);
	phy.body->SetLinearVelocity(static_cast<b2Vec2>(direction * linear_speed));

	// Add graphics
	auto& gfx = obj.add_graphic(GAME.get_sprite(explosive_weapon.game_sprite()));
	gfx.draw_angle = angle;

	// Add character
	auto& chr = obj.add_tiny_character();
	chr.add_item(GAME.get_item(ITEM_AUTOMATIC_TTL));
	chr.add_resource(RESOURCE_TTL, ttl);

	// Object detail implementation
	obj.impl = std::make_unique<ExplosiveObjectImpl>();
	auto& impl = dynamic_cast<ExplosiveObjectImpl&>(*obj.impl);

	chr.update = [=,&obj,&phy,&impl](m2::Character& chr) {
		bool state_will_explode = (impl.explosive_state == ExplosiveState::WILL_EXPLODE);
		bool ttl_expired = (impl.explosive_state == ExplosiveState::IN_FLIGHT && !chr.has_resource(RESOURCE_TTL));
		if (state_will_explode || ttl_expired) {
			LOG_DEBUG("Create explosion body");
			m2::box2d::destroy_body(phy.body);
			auto bp = m2::box2d::example_bullet_body_blueprint();
			bp.mutable_background_fixture()->mutable_circ()->set_radius(damage_radius);
			bp.mutable_background_fixture()->set_is_sensor(true);
			bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::FRIEND_OFFENSE_ON_FOREGROUND);
			phy.body = m2::box2d::create_body(*LEVEL.world, obj.physique_id(), obj.position, bp);
			// Set state
			impl.explosive_state = ExplosiveState::EXPLODING;
		}
	};
	phy.on_collision = [=,&chr,&impl](MAYBE m2::Physique& phy, m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		if (impl.explosive_state == ExplosiveState::IN_FLIGHT) {
			// May collide with multiple objects during flight, not a problem since only state is set
			LOG_DEBUG("Will explode next step");
			impl.explosive_state = ExplosiveState::WILL_EXPLODE;
		} else if (impl.explosive_state == ExplosiveState::EXPLODING) {
			auto& other_obj = other.parent();
			if (other_obj.character_id()) {
				m2::Character::execute_interaction(chr, InteractionType::COLLIDE_TO, other_obj.character(), InteractionType::GET_COLLIDED_BY);
				// TODO knock-back
			}
		} else {
			throw M2ERROR("Invalid explosive state");
		}
	};
	chr.interact = [=](m2::Character& self, m2::Character& other, InteractionType interaction_type) {
		if (interaction_type == InteractionType::COLLIDE_TO) {
			auto distance = self.parent().position.distance(other.parent().position);
			auto damage_ratio = distance / damage_radius;
			if (damage_ratio < 1.1f) {
				// Calculate damage
				float damage = m2::apply_accuracy(average_damage, damage_accuracy) * damage_ratio;
				// Create and give damage item
				other.add_item(m2::make_damage_item(RESOURCE_HP, damage));
			}
		}
	};
	phy.post_step = [=,&impl](m2::Physique& phy) {
		if (impl.explosive_state == ExplosiveState::EXPLODING) {
			GAME.add_deferred_action(m2::create_object_deleter(phy.object_id));
		}
	};

	return {};
}
