#include <rpg/object/Player.h>
#include <m2/Object.h>
#include "m2/Log.h"
#include "m2/Game.h"
#include <rpg/Graphic.h>
#include <rpg/Detail.h>
#include "m2/Controls.h"
#include <m2/game/CharacterMovement.h>
#include <rpg/Objects.h>
#include <m2/M2.h>
#include <Character.pb.h>
#include <rpg/Defs.h>
#include <array>
#include <m2/third_party/physics/ColliderCategory.h>


using namespace rpg;
using namespace m2g;
using namespace m2g::pb;

rpg::Player::Player(m2::Object& obj) : ObjectImpl(obj), animation_fsm(m2g::pb::ANIMATION_TYPE_PLAYER_MOVEMENT, obj.GetGraphicId()) {}

m2::void_expected rpg::Player::init(m2::Object& obj, const m2::VecF& position) {
	auto id = obj.GetId();
	auto main_sprite_type = *M2_GAME.GetMainSpriteOfObject(m2g::pb::PLAYER);
	const auto& mainSprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(main_sprite_type));

	auto& phy = obj.AddPhysique();
	m2::third_party::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::third_party::physics::RigidBodyType::DYNAMIC,
		.fixtures = {
			m2::third_party::physics::FixtureDefinition{
				.shape = m2::third_party::physics::CircleShape::FromSpriteCircleFixture(mainSprite.OriginalPb().regular().fixtures(0).circle(), mainSprite.Ppm()),
				.colliderFilter = m2::third_party::physics::gColliderCategoryToParams[m2::I(m2::third_party::physics::ColliderCategory::COLLIDER_CATEGORY_BACKGROUND_FRIENDLY_OBJECT)]
			},
			m2::third_party::physics::FixtureDefinition{
				.shape = m2::third_party::physics::CircleShape::FromSpriteCircleFixture(mainSprite.OriginalPb().regular().fixtures(1).circle(), mainSprite.Ppm()),
				.colliderFilter = m2::third_party::physics::gColliderCategoryToParams[m2::I(m2::third_party::physics::ColliderCategory::COLLIDER_CATEGORY_FOREGROUND_FRIENDLY_OBJECT)]
			}
		},
		.linearDamping = PLAYER_LINEAR_DAMPING,
		.fixedRotation = true,
		.mass = PLAYER_MASS,
		.allowSleeping = false,
		.initiallyAwake = true,
		.isBullet = false
	};
	phy.position = position;
	phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)] = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), position, {}, m2::pb::PhysicsLayer::SEA_LEVEL);

	auto& gfx = obj.AddGraphic(m2::pb::UprightGraphicsLayer::SEA_LEVEL_UPRIGHT, main_sprite_type, position);
	gfx.position = position;

	auto& chr = obj.AddFastCharacter();
	chr.AddNamedItem(M2_GAME.GetNamedItem(m2g::pb::ITEM_REUSABLE_DASH_2S));
	if (M2_LEVEL.GetLevelIdentifier() != "MeleeTutorialClosed") {
		// 4th level is melee tutorial
		chr.AddNamedItem(M2_GAME.GetNamedItem(m2g::pb::ITEM_REUSABLE_GUN));
	}
	chr.AddNamedItem(M2_GAME.GetNamedItem(m2g::pb::ITEM_REUSABLE_SWORD));
	chr.AddNamedItem(M2_GAME.GetNamedItem(m2g::pb::ITEM_AUTOMATIC_DASH_ENERGY));
	chr.AddNamedItem(M2_GAME.GetNamedItem(m2g::pb::ITEM_AUTOMATIC_RANGED_ENERGY));
	chr.AddNamedItem(M2_GAME.GetNamedItem(m2g::pb::ITEM_AUTOMATIC_MELEE_ENERGY));
	chr.AddResource(m2g::pb::RESOURCE_HP, 1.0f);
	chr.SetMaxResource(m2g::pb::RESOURCE_HP, 1.0f);
	chr.AddResource(m2g::pb::RESOURCE_DASH_ENERGY, 2.0f);

	obj.impl = std::make_unique<rpg::Player>(obj);
	auto& impl = dynamic_cast<Player&>(*obj.impl);

	phy.preStep = [&, id=id](m2::Physique& phy, const m2::Stopwatch::Duration& delta) {
		auto& chr = obj.GetCharacter();
		auto vector_to_mouse = (M2_GAME.MousePositionWorldM() - phy.position).Normalize();

		auto [direction_enum, direction_vector] = m2::calculate_character_movement(m2g::pb::MOVE_LEFT, m2g::pb::MOVE_RIGHT, m2g::pb::MOVE_UP, m2g::pb::MOVE_DOWN);
		float move_force;
		// Check if dash
		if (direction_vector && M2_GAME.events.PopKeyPress(m2g::pb::DASH) && chr.UseItem(chr.FindItems(m2g::pb::ITEM_REUSABLE_DASH_2S))) {
			chr.ClearResource(RESOURCE_DASH_ENERGY);
			move_force = PLAYER_DASH_FORCE;
		} else {
			// Character movement
			auto anim_state_type = detail::to_animation_state_type(direction_enum);
			impl.animation_fsm.signal(m2::AnimationFsmSignal{anim_state_type});
			move_force = PLAYER_WALK_FORCE;
		}
		if (direction_vector) {
			// Apply force
			phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->ApplyForceToCenter(direction_vector * (move_force * m2::ToDurationF(delta)));
		}

		// Primary weapon
		if (M2_GAME.events.IsMouseButtonDown(m2::MouseButton::PRIMARY)) {
			auto shoot = [&](const m2::Item& weapon) {
				rpg::create_projectile(*m2::CreateObject({}, id), phy.position, vector_to_mouse, weapon, true);
				// Knock-back
				phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->ApplyForceToCenter(m2::VecF::CreateUnitVectorWithAngle(vector_to_mouse.GetAngle() + m2::PI) * 50000.0f);
			};

			// Check if there is a special ranged weapon and try to use the item
			auto special_it = chr.FindItems(m2g::pb::ITEM_CATEGORY_SPECIAL_RANGED_WEAPON);
			if (special_it) {
				if (chr.UseItem(special_it)) {
					shoot(*special_it);
				}
			} else {
				// Find default weapon and try to use it
				auto default_it = chr.FindItems(m2g::pb::ITEM_CATEGORY_DEFAULT_RANGED_WEAPON);
				if (default_it && chr.UseItem(default_it)) {
					chr.ClearResource(RESOURCE_RANGED_ENERGY);
					shoot(*default_it);
				}
			}
		}

		// Secondary weapon
		if (M2_GAME.events.IsMouseButtonDown(m2::MouseButton::SECONDARY)) {
			auto slash = [&](const m2::Item& weapon) {
				rpg::create_blade(*m2::CreateObject({}, id), phy.position, vector_to_mouse, weapon, true);
			};

			// Check if there is a special melee weapon and try to use the item
			auto special_it = chr.FindItems(m2g::pb::ITEM_CATEGORY_SPECIAL_MELEE_WEAPON);
			if (special_it) {
				// Try to use the weapon
				if (chr.UseItem(special_it)) {
					slash(*special_it);
				}
			} else {
				// Find default melee weapon and try to use it
				auto default_it = chr.FindItems(m2g::pb::ITEM_CATEGORY_DEFAULT_MELEE_WEAPON);
				if (default_it && chr.UseItem(default_it)) {
					chr.ClearResource(RESOURCE_MELEE_ENERGY);
					slash(*default_it);
				}
			}
		}
	};
	chr.update = [](MAYBE m2::Character& chr, const m2::Stopwatch::Duration&) {
		// Check if died
		if (not chr.HasResource(m2g::pb::RESOURCE_HP)) {
			LOG_INFO("You died");
			if (m2::UiPanel::create_and_run_blocking(M2G_PROXY.you_died_menu()).IsQuit()) {
				M2_GAME.quit = true;
			}
		}
		// Check if special ammo finished
		if (auto special_it = chr.FindItems(m2g::pb::ITEM_CATEGORY_SPECIAL_RANGED_WEAPON);
			special_it && !chr.HasResource(m2g::pb::RESOURCE_SPECIAL_RANGED_WEAPON_AMMO)) {
			// Remove weapon if no ammo left
			chr.RemoveItem(special_it);
		}
		// Show/hide ammo display
		M2G_PROXY.set_ammo_display_state((bool) chr.FindItems(m2g::pb::ITEM_CATEGORY_SPECIAL_RANGED_WEAPON));
	};
	phy.onCollision = [](MAYBE m2::Physique& me, m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		if (auto* other_char = other.Owner().TryGetCharacter(); other_char && 10.0f < m2::VecF{me.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->GetLinearVelocity()}.GetLength()) {
			InteractionData data;
			data.set_stun_duration(2.0f);
			other_char->ExecuteInteraction(data);
		}
	};
	chr.on_interaction = [](m2::Character& self, MAYBE m2::Character* other, const InteractionData& data) -> std::optional<m2g::pb::InteractionData> {
		if (data.has_hit_damage()) {
			// Get hit by an enemy
			self.RemoveResource(m2g::pb::RESOURCE_HP, data.hit_damage());
		} else if (data.has_item_type()) {
			const auto& item = M2_GAME.GetNamedItem(data.item_type());
			// Player can hold only one special weapon of certain type, get rid of the previous one
			constexpr std::array<ItemCategory, 2> special_categories = {ITEM_CATEGORY_SPECIAL_RANGED_WEAPON, ITEM_CATEGORY_SPECIAL_MELEE_WEAPON};
			constexpr std::array<ResourceType, 2> special_ammo_type = {RESOURCE_SPECIAL_RANGED_WEAPON_AMMO, NO_RESOURCE};
			for (size_t i = 0; i < special_categories.size(); ++i) {
				if (auto sp = special_categories[i]; sp == item.Category()) {
					if (auto it = self.FindItems(sp); it) {
						self.RemoveItem(it); // Remove weapon
						self.ClearResource(special_ammo_type[i]); // Also remove any ammo
					}
					break;
				}
			}
			// Add item
			self.AddNamedItem(item);
		}
		return std::nullopt;
	};
	gfx.preDraw = [&](MAYBE m2::Graphic& gfx, const m2::Stopwatch::Duration& delta) {
		impl.animation_fsm.time(m2::ToDurationF(delta));
	};
	gfx.onDraw = [&](m2::Graphic& gfx) {
		// Draw the sprite itself
		m2::Graphic::DefaultDrawCallback(gfx);
		// Draw the HP addon
		DrawAddons(gfx, chr.GetResource(RESOURCE_HP));
	};

	M2_LEVEL.playerId = M2_LEVEL.objects.GetId(&obj);
	return {};
}
