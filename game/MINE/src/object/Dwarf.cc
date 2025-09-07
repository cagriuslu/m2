#include <mine/object/Dwarf.h>
#include <m2/Game.h>
#include <m2g_SpriteType.pb.h>
#include <m2/box2d/Query.h>
#include <m2/game/CharacterMovement.h>
#include <m2/third_party/physics/ColliderCategory.h>

using namespace m2g;
using namespace m2g::pb;
using namespace m2::third_party::physics;

m2::void_expected create_dwarf(m2::Object& obj) {
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(DWARF_FULL));

	auto& phy = obj.AddPhysique();
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
	phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)] = RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), obj.position, obj.orientation, m2::pb::PhysicsLayer::SEA_LEVEL);

	obj.AddGraphic(m2::pb::UprightGraphicsLayer::SEA_LEVEL_UPRIGHT, DWARF_FULL);

	auto& chr = obj.AddFastCharacter();
	chr.AddNamedItem(M2_GAME.GetNamedItem(ITEM_REUSABLE_JUMP));
	chr.AddNamedItem(M2_GAME.GetNamedItem(ITEM_AUTOMATIC_JUMP_ENERGY));

	phy.preStep = [&obj, &chr](m2::Physique& phy, const m2::Stopwatch::Duration& delta) {
		// Character movement
		auto [direction_enum, direction_vector] = m2::calculate_character_movement(MOVE_LEFT, MOVE_RIGHT, NO_KEY, NO_KEY);
		if (direction_enum == m2::CHARMOVEMENT_NONE) {
			// Slow down character
			auto linear_velocity = phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->GetLinearVelocity();
			if (0.0f < abs(linear_velocity.x)) {
				linear_velocity.x /= 1.25f;
				phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->SetLinearVelocity(linear_velocity);
			}
		} else {
			// Accelerate character
			auto force_multiplier = m2::CalculateLimitedForce(phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->GetLinearVelocity().x, 5.0f);
			phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->ApplyForceToCenter(direction_vector * force_multiplier * 4000.0f);
		}
		// Jump
		auto is_grounded = chr.GetResource(RESOURCE_IS_GROUNDED_X) != 0.0f && chr.GetResource(RESOURCE_IS_GROUNDED_Y) != 0.0f;
		if (is_grounded && M2_GAME.events.IsKeyDown(JUMP) && chr.UseItem(chr.FindItems(ITEM_REUSABLE_JUMP))) {
			auto linear_velocity = phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->GetLinearVelocity();
			linear_velocity.y -= 7.0f;
			phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->SetLinearVelocity(linear_velocity);
		}

		// Mouse button
		if (M2_GAME.events.IsMouseButtonDown(m2::MouseButton::PRIMARY)) {
			m2::box2d::FindObjectsNearPositionUnderMouse(obj.position, 2.0f, [&delta](m2::Physique& other_phy) -> bool {
				auto& obj_under_mouse = other_phy.Owner();
				// If object under mouse has character
				if (obj_under_mouse.GetCharacterId()) {
					auto& chr_under_mouse = obj_under_mouse.GetCharacter();
					// If character has HP
					if (chr_under_mouse.HasResource(RESOURCE_HP)) {
						// Damage object
						chr_under_mouse.RemoveResource(RESOURCE_HP, 2.0f * m2::ToDurationF(delta));
						// Show health bar
						auto hp = chr_under_mouse.GetResource(RESOURCE_HP);
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
	phy.onCollision = [&chr](MAYBE m2::Physique& phy, m2::Physique& other, const m2::box2d::Contact& contact) {
		// Check if in contact with obstacle
		if (other.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)] && other.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->GetAllLayersBelongingTo() & (COLLIDER_LAYER_BACKGROUND_OBSTACLE | COLLIDER_LAYER_FOREGROUND_OBSTACLE)) {
			// Check is contact normal points upwards
			if (abs(contact.normal.x) <= -contact.normal.y) {
				chr.SetResource(RESOURCE_IS_GROUNDED_X, other.Owner().position.x);
				chr.SetResource(RESOURCE_IS_GROUNDED_Y, other.Owner().position.y);
			}
		}
	};
	phy.offCollision = [&chr](MAYBE m2::Physique& phy, m2::Physique& other) {
		// Check if in contact with obstacle
		if (other.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)] && other.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->GetAllLayersBelongingTo() & (COLLIDER_LAYER_BACKGROUND_OBSTACLE | COLLIDER_LAYER_FOREGROUND_OBSTACLE)) {
			// Check if the other object is the grounding object
			if (chr.GetResource(RESOURCE_IS_GROUNDED_X) == other.Owner().position.x && chr.GetResource(RESOURCE_IS_GROUNDED_Y) == other.Owner().position.y) {
				chr.SetResource(RESOURCE_IS_GROUNDED_X, 0.0f);
				chr.SetResource(RESOURCE_IS_GROUNDED_Y, 0.0f);
			}
		}
	};

	M2_LEVEL.playerId = obj.GetId();
	return {};
}
