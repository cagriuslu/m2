#include <m2/Object.h>
#include "m2/Game.h"
#include <rpg/object/Projectile.h>
#include <m2/box2d/Detail.h>
#include <m2/M2.h>
#include <m2/Log.h>

using namespace m2g;
using namespace m2g::pb;

// TODO add other types of Ranged Weapons: Machine Gun, Shotgun, Bow

m2::VoidValue rpg::create_projectile(m2::Object& obj, const m2::VecF& intended_direction, const m2::Item& ranged_weapon, bool is_friend) {
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
	float damage_radius = ranged_weapon.try_get_attribute(ATTRIBUTE_DAMAGE_RADIUS, 0.0f);
	bool is_explosive = (0.0f < damage_radius);
	float average_ttl = ranged_weapon.get_attribute(ATTRIBUTE_AVERAGE_TTL);
	float ttl_accuracy = ranged_weapon.try_get_attribute(ATTRIBUTE_TTL_ACCURACY, 1.0f);

	float angle = m2::apply_accuracy(intended_direction.angle_rads(), m2::PI, angular_accuracy);
	auto direction = m2::VecF::from_angle(angle);
	float ttl = m2::apply_accuracy(average_ttl, average_ttl, ttl_accuracy);

	const auto& sprite = GAME.get_sprite(ranged_weapon.game_sprite());

	// Add physics
	auto& phy = obj.add_physique();
	auto bp = m2::box2d::example_bullet_body_blueprint();
	bp.mutable_foreground_fixture()->mutable_circ()->mutable_center_offset()->set_x(sprite.foreground_collider_center_offset_m().x);
	bp.mutable_foreground_fixture()->mutable_circ()->mutable_center_offset()->set_y(sprite.foreground_collider_center_offset_m().y);
	bp.mutable_foreground_fixture()->mutable_circ()->set_radius(sprite.foreground_collider_circ_radius_m());
	bp.mutable_foreground_fixture()->set_is_sensor(true);
	bp.mutable_foreground_fixture()->set_category(is_friend ? m2::pb::FixtureCategory::FRIEND_OFFENSE_ON_FOREGROUND : m2::pb::FixtureCategory::FOE_OFFENSE_ON_FOREGROUND);
	bp.set_fixed_rotation(true);
	phy.body = m2::box2d::create_body(*LEVEL.world, obj.physique_id(), obj.position, bp);
	phy.body->SetLinearVelocity(static_cast<b2Vec2>(direction * linear_speed));

	// Add graphics
	auto& gfx = obj.add_graphic(sprite);
	gfx.draw_angle = angle;
	gfx.z = 0.5f;

	// Add character
	auto& chr = obj.add_tiny_character();
	chr.add_item(GAME.get_item(ITEM_AUTOMATIC_TTL));
	chr.add_resource(RESOURCE_TTL, ttl);

	chr.update = [=, &phy, &obj](m2::Character& chr) {
		if (!chr.has_resource(RESOURCE_TTL)) {
			if (is_explosive) {
				LOG_DEBUG("Exploding...");
				auto bp = m2::box2d::example_bullet_body_blueprint();
				bp.mutable_background_fixture()->mutable_circ()->set_radius(damage_radius);
				bp.mutable_background_fixture()->set_is_sensor(true);
				bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::FRIEND_OFFENSE_ON_FOREGROUND);
				phy.body = m2::box2d::create_body(*LEVEL.world, obj.physique_id(), obj.position, bp);
				chr.add_item(GAME.get_item(ITEM_AUTOMATIC_EXPLOSIVE_TTL));
				// RESOURCE_EXPLOSION_TTL only means the object is currently exploding
				chr.set_resource(RESOURCE_EXPLOSION_TTL, 1.0f); // 1.0f is just symbolic
			} else {
				LOG_DEBUG("Destroying self");
				GAME.add_deferred_action(m2::create_object_deleter(chr.object_id));
			}
		}
	};
	phy.on_collision = [=, &chr](MAYBE m2::Physique& phy, m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		if (is_explosive && chr.has_resource(RESOURCE_TTL)) {
			LOG_DEBUG("Explosive hit a target during flight, will explode next step");
			chr.set_resource(RESOURCE_TTL, 0.0f); // Clear TTL, chr.update will create the explosion
		} else {
			if (other.parent().character_id()) {
				LOG_DEBUG("Hit a target, triggering interaction...");
				m2::Character::execute_interaction(chr, other.parent().character(), InteractionType::HIT);
				// TODO knock-back
			}
		}
	};
	chr.create_interaction = [=](m2::Character& self, m2::Character& other, m2g::pb::InteractionType type) -> std::optional<m2g::pb::InteractionData> {
		if (type == InteractionType::HIT) {
			InteractionData data;
			if (is_explosive && self.has_resource(RESOURCE_EXPLOSION_TTL)) {
				LOG_DEBUG("Explosive damage");
				auto distance = self.parent().position.distance(other.parent().position);
				auto damage_ratio = distance / damage_radius;
				if (damage_ratio < 1.1f) {
					// Calculate damage
					float damage = m2::apply_accuracy(average_damage, average_damage, damage_accuracy) * damage_ratio;
					data.set_hit_damage(damage);
				}
			} else if (self.has_resource(RESOURCE_TTL)) {
				LOG_DEBUG("Regular damage");
				// Calculate damage
				float damage = m2::apply_accuracy(average_damage, average_damage, damage_accuracy);
				data.set_hit_damage(damage);
				// Clear TTL
				self.clear_resource(RESOURCE_TTL);
			}
			return data;
		}
		return std::nullopt;
	};
	phy.post_step = [&chr](m2::Physique& phy) {
		if (chr.has_resource(RESOURCE_EXPLOSION_TTL)) {
			LOG_DEBUG("Exploded");
			GAME.add_deferred_action(m2::create_object_deleter(phy.object_id));
		}
	};

	return {};
}
