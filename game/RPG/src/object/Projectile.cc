#include <m2/ObjectEx.h>
#include <m2/Object.h>
#include <rpg/Physics.h>
#include "m2/Game.h"
#include <rpg/Objects.h>
#include <m2/M2.h>
#include <m2/Log.h>
#include <m2/third_party/physics/ColliderCategory.h>

using namespace m2g;
using namespace m2g::pb;

// TODO add other types of Ranged Weapons: Machine Gun, Shotgun, Bow

m2::void_expected rpg::create_projectile(m2::Object& obj, const m2::VecF& position, const m2::VecF& intended_direction, const m2::Card& ranged_weapon, bool is_friend) {
	// Check if weapon has necessary attributes
	if (!ranged_weapon.GetConstant(ATTRIBUTE_LINEAR_SPEED)) {
		throw M2_ERROR("Ranged weapon has no linear speed");
	}
	if (!ranged_weapon.GetConstant(ATTRIBUTE_AVERAGE_DAMAGE)) {
		throw M2_ERROR("Ranged weapon has no average damage");
	}
	if (!ranged_weapon.GetConstant(ATTRIBUTE_AVERAGE_TTL)) {
		throw M2_ERROR("Ranged weapon has no average TTL");
	}

	float linear_speed = ranged_weapon.GetConstant(ATTRIBUTE_LINEAR_SPEED).GetFOrZero();
	float angular_accuracy = ranged_weapon.GetConstant(ATTRIBUTE_ANGULAR_ACCURACY).GetFOrValue(1.0f);
	float average_damage = ranged_weapon.GetConstant(ATTRIBUTE_AVERAGE_DAMAGE).GetFOrZero();
	float damage_accuracy = ranged_weapon.GetConstant(ATTRIBUTE_DAMAGE_ACCURACY).GetFOrValue(1.0f);
	float damage_radius = ranged_weapon.GetConstant(ATTRIBUTE_DAMAGE_RADIUS).GetFOrZero();
	bool is_explosive = (0.0f < damage_radius);
	float average_ttl = ranged_weapon.GetConstant(ATTRIBUTE_AVERAGE_TTL).GetFOrZero();
	float ttl_accuracy = ranged_weapon.GetConstant(ATTRIBUTE_TTL_ACCURACY).GetFOrValue(1.0f);

	float angle = m2::ApplyAccuracy(intended_direction.GetAngle(), m2::PI, angular_accuracy);
	auto direction = m2::VecF::CreateUnitVectorWithAngle(angle);
	float ttl = m2::ApplyAccuracy(average_ttl, average_ttl, ttl_accuracy);

	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(ranged_weapon.GameSprite()));

	// Add physics
	auto& phy = obj.AddPhysique();
	phy.position = position;
	phy.orientation = m2::FE{angle};
	auto rigidBodyDef = BasicBulletRigidBodyDefinition();
	rigidBodyDef.fixtures = {m2::third_party::physics::FixtureDefinition{
		.shape = m2::third_party::physics::CircleShape::FromSpriteCircleFixture(sprite.OriginalPb().regular().fixtures(0).circle(), sprite.Ppm()),
		.isSensor = true,
		.colliderFilter = m2::third_party::physics::gColliderCategoryToParams[m2::I(is_friend
				? m2::third_party::physics::ColliderCategory::COLLIDER_CATEGORY_FOREGROUND_FRIENDLY_DAMAGE
				: m2::third_party::physics::ColliderCategory::COLLIDER_CATEGORY_FOREGROUND_HOSTILE_DAMAGE)]
	}};
	phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)] = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), position, angle, m2::pb::PhysicsLayer::SEA_LEVEL);
	phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->SetLinearVelocity(direction * linear_speed);

	// Add graphics
	auto& gfx = obj.AddGraphic(m2::pb::UprightGraphicsLayer::SEA_LEVEL_UPRIGHT, ranged_weapon.GameSprite());
	gfx.position = position;
	gfx.orientation = angle;
	gfx.z = 0.5f;

	// Add character
	auto& chr = m2::AddCharacterToObject<m2g::ProxyEx::CompactCharacterStorageIndex>(obj);
	chr.UnsafeSetVariable(RESOURCE_TTL, ttl);

	chr.update = [=, &phy, &obj](m2::Character& chr, const m2::Stopwatch::Duration& delta) {
		chr.UnsafeSubtractVariable(RESOURCE_TTL, std::chrono::duration_cast<std::chrono::duration<float>>(delta).count(), 0.0f);

		if (not chr.GetVariable(RESOURCE_TTL)) {
			if (is_explosive) {
				LOG_DEBUG("Exploding...");
				auto explosionBodyDef = BasicBulletRigidBodyDefinition();
				explosionBodyDef.fixtures = {m2::third_party::physics::FixtureDefinition{
					.shape = m2::third_party::physics::CircleShape{.radius = damage_radius},
					.isSensor = true,
					.colliderFilter = m2::third_party::physics::gColliderCategoryToParams[m2::I(m2::third_party::physics::ColliderCategory::COLLIDER_CATEGORY_FOREGROUND_FRIENDLY_DAMAGE)]
				}};
				phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)] = m2::third_party::physics::RigidBody::CreateFromDefinition(explosionBodyDef, obj.GetPhysiqueId(), obj.GetPhysique().position, phy.orientation.ToFloat(), m2::pb::PhysicsLayer::SEA_LEVEL);
				// RESOURCE_EXPLOSION_TTL only means the object is currently exploding
				chr.UnsafeSetVariable(RESOURCE_EXPLOSION_TTL, 1.0f); // 1.0f is just symbolic
			} else {
				LOG_DEBUG("Destroying self");
				M2_DEFER(m2::CreateObjectDeleter(chr.OwnerId()));
			}
		}
	};
	phy.onCollision = [=, &chr](MAYBE m2::Physique& phy, m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		if (is_explosive && chr.GetVariable(RESOURCE_TTL)) {
			LOG_DEBUG("Explosive hit a target during flight, will explode next step");
			chr.ClearVariable(RESOURCE_TTL); // Clear TTL, chr.update will create the explosion
		} else {
			if (auto* other_char = other.Owner().TryGetCharacter(); other_char) {
				float damage = 0.0f;
				if (is_explosive && chr.GetVariable(RESOURCE_EXPLOSION_TTL)) {
					LOG_DEBUG("Explosive damage");
					auto distance = chr.Owner().GetPhysique().position.GetDistanceTo(other.position);
					auto damage_ratio = distance / damage_radius;
					if (damage_ratio < 1.1f) {
						damage = m2::ApplyAccuracy(average_damage, average_damage, damage_accuracy) * damage_ratio;
					}
				} else if (chr.GetVariable(RESOURCE_TTL)) {
					LOG_DEBUG("Regular damage");
					damage = m2::ApplyAccuracy(average_damage, average_damage, damage_accuracy);
					chr.ClearVariable(RESOURCE_TTL);
				}
				other_char->ExecuteInteraction(std::make_unique<m2g::Proxy::HitDamage>(damage));
			}
		}
	};
	phy.postStep = [&chr](m2::Physique& phy, const m2::Stopwatch::Duration&) {
		if (chr.GetVariable(RESOURCE_EXPLOSION_TTL)) {
			LOG_DEBUG("Exploded");
			M2_DEFER(m2::CreateObjectDeleter(phy.OwnerId()));
		}
	};

	return {};
}
