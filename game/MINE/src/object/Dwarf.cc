#include <mine/object/Dwarf.h>
#include <m2/Game.h>
#include <m2g_SpriteType.pb.h>
#include <m2/box2d/Query.h>
#include <m2/game/CharacterMovement.h>
#include <m2/third_party/physics/ColliderCategory.h>
#include <m2/ObjectEx.h>

using namespace m2g;
using namespace m2g::pb;
using namespace m2::third_party::physics;

m2::void_expected create_dwarf(m2::Object& obj, const m2::VecF& position) {
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(DWARF_FULL));

	auto& phy = obj.AddPhysique();
	phy.position = position;
	RigidBodyDefinition rigidBodyDef{
		.bodyType = RigidBodyType::DYNAMIC,
		.fixtures = {FixtureDefinition{
			.shape = ToShape(sprite.OriginalPb().regular().fixtures(0), sprite.Ppm()),
			.friction = 0.0f,
			.restitution = 0.0f,
			.colliderFilter = gColliderCategoryToParams[m2::I(ColliderCategory::COLLIDER_CATEGORY_BACKGROUND_FRIENDLY_OBJECT)]
		}},
		.linearDamping = 0.0f,
		.fixedRotation = true,
		.mass = 100.0f,
		.gravityScale = 2.0f,
		.allowSleeping = false,
		.initiallyAwake = true,
		.isBullet = false,
		.initiallyEnabled = true
	};
	phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)] = RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), position, {}, m2::pb::PhysicsLayer::SEA_LEVEL);

	obj.AddGraphic(m2::pb::UprightGraphicsLayer::SEA_LEVEL_UPRIGHT, DWARF_FULL).position = position;

	auto& chr = m2::AddCharacterToObject<ProxyEx::FastCharacterStorageIndex>(obj);

	phy.preStep = [&chr](m2::Physique& phy, const m2::Stopwatch::Duration& delta) {
		// Character movement
		auto [direction_enum, direction_vector] = m2::calculate_character_movement(MOVE_LEFT, MOVE_RIGHT, NO_KEY, NO_KEY);
		if (direction_enum == m2::CHARMOVEMENT_NONE) {
			// Slow down character
			auto linear_velocity = phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->GetLinearVelocity();
			if (0.0f < abs(linear_velocity.GetX())) {
				linear_velocity = linear_velocity.WithX(linear_velocity.GetX() / 1.25f);
				phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->SetLinearVelocity(linear_velocity);
			}
		} else {
			// Accelerate character
			auto force_multiplier = m2::CalculateLimitedForce(phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->GetLinearVelocity().GetX(), 5.0f);
			phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->ApplyForceToCenter(direction_vector * force_multiplier * 4000.0f);
		}
		// Jump
		const auto is_grounded = static_cast<bool>(chr.GetVariable(VARIABLE_IS_GROUNDED_X))
			&& static_cast<bool>(chr.GetVariable(VARIABLE_IS_GROUNDED_Y));
		if (is_grounded && M2_GAME.events.IsKeyDown(JUMP)) {
			chr.ClearVariable(VARIABLE_JUMP_ENERGY);
			auto linear_velocity = phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->GetLinearVelocity();
			linear_velocity = linear_velocity.WithY(linear_velocity.GetY() - 7.0f);
			phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->SetLinearVelocity(linear_velocity);
		}

		// Mouse button
		if (M2_GAME.events.IsMouseButtonDown(m2::MouseButton::PRIMARY)) {
			m2::box2d::FindObjectsNearPositionUnderMouse(phy.position, 2.0f, [&delta](m2::Physique& other_phy) -> bool {
				auto& obj_under_mouse = other_phy.Owner();
				// If object under mouse has character
				if (obj_under_mouse.GetCharacterId()) {
					auto& chr_under_mouse = obj_under_mouse.GetCharacter();
					// If character has HP
					if (chr_under_mouse.GetVariable(VARIABLE_HP)) {
						// Damage object
						chr_under_mouse.SubtractVariable(VARIABLE_HP, 2.0f * m2::ToDurationF(delta), 0.0f);
						// Show health bar
						auto hp = chr_under_mouse.GetVariable(VARIABLE_HP).GetFEOrZero().ToFloat();
						// If object under mouse runs out of HP
						if (hp == 0.0f) {
							// Delete object
							M2_DEFER(m2::CreateObjectDeleter(chr_under_mouse.OwnerId()));
						}
					}
					// Stop searching
					return false;
				}
				// Continue searching
				return true;
			});
		}
	};
	chr.update = [](m2::Character& chr, const m2::Stopwatch::Duration& delta) {
		chr.AddVariable(VARIABLE_JUMP_ENERGY, std::chrono::duration_cast<std::chrono::duration<float>>(delta).count());
	};
	phy.onCollision = [&chr](MAYBE m2::Physique& phy, m2::Physique& other, const m2::box2d::Contact& contact) {
		// Check if in contact with obstacle
		if (other.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)] && other.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->GetAllLayersBelongingTo() & (COLLIDER_LAYER_BACKGROUND_OBSTACLE | COLLIDER_LAYER_FOREGROUND_OBSTACLE)) {
			// Check is contact normal points upwards
			if (abs(contact.normal.GetX()) <= -contact.normal.GetY()) {
				chr.SetVariable(VARIABLE_IS_GROUNDED_X, other.position.GetX());
				chr.SetVariable(VARIABLE_IS_GROUNDED_Y, other.position.GetY());
			}
		}
	};
	phy.offCollision = [&chr](MAYBE m2::Physique& phy, m2::Physique& other) {
		// Check if in contact with obstacle
		if (other.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)] && other.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->GetAllLayersBelongingTo() & (COLLIDER_LAYER_BACKGROUND_OBSTACLE | COLLIDER_LAYER_FOREGROUND_OBSTACLE)) {
			// Check if the other object is the grounding object
			if (chr.GetVariable(VARIABLE_IS_GROUNDED_X).GetFOrZero() == other.position.GetX() && chr.GetVariable(VARIABLE_IS_GROUNDED_Y).GetFOrZero() == other.position.GetY()) {
				chr.SetVariable(VARIABLE_IS_GROUNDED_X, 0.0f);
				chr.SetVariable(VARIABLE_IS_GROUNDED_Y, 0.0f);
			}
		}
	};

	M2_LEVEL.playerId = obj.GetId();
	return {};
}
