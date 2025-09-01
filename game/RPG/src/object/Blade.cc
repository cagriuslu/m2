#include <m2/math/VecF.h>
#include <m2/Object.h>
#include "m2/Game.h"
#include <rpg/Objects.h>
#include <m2/Log.h>
#include <m2/third_party/physics/ColliderCategory.h>
#include <rpg/Physics.h>

using namespace m2g;
using namespace m2g::pb;

m2::void_expected rpg::create_blade(m2::Object &obj, const m2::VecF &direction, const m2::Item &melee_weapon, bool is_friend) {
	// Check if weapon has necessary attributes
	if (!melee_weapon.HasAttribute(ATTRIBUTE_AVERAGE_DAMAGE)) {
		throw M2_ERROR("Melee weapon has no average damage");
	}
	if (!melee_weapon.HasAttribute(ATTRIBUTE_AVERAGE_TTL)) {
		throw M2_ERROR("Melee weapon has no average TTL");
	}
	float average_damage = melee_weapon.GetAttribute(ATTRIBUTE_AVERAGE_DAMAGE);
	float damage_accuracy = melee_weapon.TryGetAttribute(ATTRIBUTE_DAMAGE_ACCURACY, 1.0f);
	float average_ttl = melee_weapon.GetAttribute(ATTRIBUTE_AVERAGE_TTL);

	const float direction_angle = direction.GetAngle();
	constexpr float swing_angle = m2::ToRadians(120.0f); // Swing angle is 120 degrees
	const float start_angle = direction_angle + swing_angle / 2.0f;
	const float swing_speed = swing_angle / average_ttl;

	obj.orientation = start_angle;

	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(melee_weapon.GameSprite()));

	// Add physics
	auto& phy = obj.AddPhysique();

	auto rigidBodyDef = BasicBulletRigidBodyDefinition();
	rigidBodyDef.fixtures = {m2::third_party::physics::FixtureDefinition{
		.shape = m2::third_party::physics::RectangleShape::FromSpriteRectangleFixture(sprite.OriginalPb().regular().fixtures(0).rectangle(), sprite.Ppm()),
		.restitution = 0.0f,
		.isSensor = true,
		.colliderFilter = m2::third_party::physics::gColliderCategoryToParams[m2::I(is_friend
			? m2::third_party::physics::ColliderCategory::COLLIDER_CATEGORY_FOREGROUND_FRIENDLY_DAMAGE
			: m2::third_party::physics::ColliderCategory::COLLIDER_CATEGORY_FOREGROUND_HOSTILE_DAMAGE)]
	}};
	phy.body[I(m2::PhysicsLayer::P0)] = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), obj.position, start_angle, m2::PhysicsLayer::P0);
	phy.body[I(m2::PhysicsLayer::P0)]->SetAngularVelocity(-swing_speed);

	// Add graphics
	auto& gfx = obj.AddGraphic(m2::ForegroundDrawLayer::F0_BOTTOM, melee_weapon.GameSprite());
	gfx.z = 0.5f;

	// Add character
	auto& chr = obj.AddTinyCharacter();
	chr.AddNamedItem(M2_GAME.GetNamedItem(ITEM_AUTOMATIC_TTL));
	chr.AddResource(RESOURCE_TTL, average_ttl);

	chr.update = [](m2::Character& chr, const m2::Stopwatch::Duration&) {
		if (!chr.HasResource(RESOURCE_TTL)) {
			M2_DEFER(m2::CreateObjectDeleter(chr.OwnerId()));
		}
	};
	phy.onCollision = [average_damage, damage_accuracy](MAYBE m2::Physique& phy, m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		if (auto* other_char = other.Owner().TryGetCharacter(); other_char) {
			InteractionData data;
			data.set_hit_damage(m2::ApplyAccuracy(average_damage, average_damage, damage_accuracy));
			other_char->ExecuteInteraction(data);
			// TODO knock-back
		}
	};
	phy.postStep = [&](m2::Physique& phy, const m2::Stopwatch::Duration&) {
		if (auto* originator = obj.TryGetParent()) {
			phy.body[I(m2::PhysicsLayer::P0)]->SetPosition(originator->position);
		} else {
			// Originator died
			M2_DEFER(m2::CreateObjectDeleter(phy.OwnerId()));
		}
	};

	return {};
}
