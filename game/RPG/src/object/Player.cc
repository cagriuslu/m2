#include <rpg/object/Player.h>
#include <m2/ObjectEx.h>
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

	auto& chr = m2::AddCharacterToObject<m2g::ProxyEx::FastCharacterStorageIndex>(obj);
	if (M2_LEVEL.GetLevelIdentifier() != "MeleeTutorialClosed") {
		// 4th level is melee tutorial
		chr.AddCard(CARD_REUSABLE_GUN);
	}
	chr.AddCard(CARD_REUSABLE_SWORD);
	chr.SetVariable(RESOURCE_HP, 1.0f);
	chr.SetVariable(RESOURCE_DASH_ENERGY, 2.0f);

	obj.impl = std::make_unique<Player>(obj);
	auto& impl = dynamic_cast<Player&>(*obj.impl);

	phy.preStep = [&, id=id](m2::Physique& phy, const m2::Stopwatch::Duration& delta) {
		auto& chr = obj.GetCharacter();
		auto vector_to_mouse = (M2_GAME.MousePositionWorldM() - phy.position).Normalize();

		auto [direction_enum, direction_vector] = m2::calculate_character_movement(MOVE_LEFT, MOVE_RIGHT, MOVE_UP, MOVE_DOWN);
		float move_force{};
		// Check if dash
		if (direction_vector && M2_GAME.events.PopKeyPress(m2g::pb::DASH)) {
			if (2.0f < chr.GetVariable(RESOURCE_DASH_ENERGY).GetFOrZero()) {
				chr.ClearVariable(RESOURCE_DASH_ENERGY);
				move_force = PLAYER_DASH_FORCE;
			}
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
			auto shoot = [&](const m2::Card& weapon) {
				rpg::create_projectile(*m2::CreateObject({}, id), phy.position, vector_to_mouse, weapon, true);
				// Knock-back
				phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->ApplyForceToCenter(m2::VecF::CreateUnitVectorWithAngle(vector_to_mouse.GetAngle() + m2::PI) * 50000.0f);
			};

			// Check if there is a special ranged weapon and try to use the card
			if (const auto* special = dynamic_cast<const m2::FastCharacter&>(chr).GetFirstCard(CARD_CATEGORY_SPECIAL_RANGED_WEAPON);
				special && special->GetConstant(CONSTANT_MELEE_ENERGY_REQUIREMENT).GetFOrZero() <= chr.GetVariable(RESOURCE_MELEE_ENERGY).GetFOrZero()) {
				chr.ClearVariable(RESOURCE_RANGED_ENERGY);
				shoot(*special);
			} else {
				// Find default weapon and try to use it
				if (const auto* defaultWeapon = dynamic_cast<const m2::FastCharacter&>(chr).GetFirstCard(CARD_CATEGORY_DEFAULT_RANGED_WEAPON);
					defaultWeapon && defaultWeapon->GetConstant(CONSTANT_RANGED_ENERGY_REQUIREMENT).GetFOrZero() <= chr.GetVariable(RESOURCE_RANGED_ENERGY).GetFOrZero()) {
					chr.ClearVariable(RESOURCE_RANGED_ENERGY);
					shoot(*defaultWeapon);
				}
			}
		}

		// Secondary weapon
		if (M2_GAME.events.IsMouseButtonDown(m2::MouseButton::SECONDARY)) {
			auto slash = [&](const m2::Card& weapon) {
				rpg::create_blade(*m2::CreateObject({}, id), phy.position, vector_to_mouse, weapon, true);
			};

			// Check if there is a special melee weapon and try to use the card
			if (const auto* special = dynamic_cast<const m2::FastCharacter&>(chr).GetFirstCard(CARD_CATEGORY_SPECIAL_MELEE_WEAPON);
				special && special->GetConstant(CONSTANT_MELEE_ENERGY_REQUIREMENT).GetFOrZero() <= chr.GetVariable(RESOURCE_MELEE_ENERGY).GetFOrZero()) {
				chr.ClearVariable(RESOURCE_MELEE_ENERGY);
				slash(*special);
			} else {
				// Find default melee weapon and try to use it
				if (const auto* defaultWeapon = dynamic_cast<const m2::FastCharacter&>(chr).GetFirstCard(CARD_CATEGORY_DEFAULT_MELEE_WEAPON);
					defaultWeapon && defaultWeapon->GetConstant(CONSTANT_MELEE_ENERGY_REQUIREMENT).GetFOrZero() <= chr.GetVariable(RESOURCE_MELEE_ENERGY).GetFOrZero()) {
					chr.ClearVariable(RESOURCE_MELEE_ENERGY);
					slash(*defaultWeapon);
				}
			}
		}
	};
	chr.update = [](MAYBE m2::Character& chr, const m2::Stopwatch::Duration& delta) {
		chr.AddVariable(RESOURCE_DASH_ENERGY, std::chrono::duration_cast<std::chrono::duration<float>>(delta).count());
		chr.AddVariable(RESOURCE_RANGED_ENERGY, std::chrono::duration_cast<std::chrono::duration<float>>(delta).count());
		chr.AddVariable(RESOURCE_MELEE_ENERGY, std::chrono::duration_cast<std::chrono::duration<float>>(delta).count());

		// Check if died
		if (not chr.GetVariable(m2g::pb::RESOURCE_HP)) {
			LOG_INFO("You died");
			if (m2::UiPanel::create_and_run_blocking(M2G_PROXY.you_died_menu()).IsQuit()) {
				M2_GAME.quit = true;
			}
		}
		// Check if special ammo finished
		if (const auto* special = dynamic_cast<const m2::FastCharacter&>(chr).GetFirstCard(CARD_CATEGORY_SPECIAL_RANGED_WEAPON);
			special && not chr.GetVariable(m2g::pb::RESOURCE_SPECIAL_RANGED_WEAPON_AMMO)) {
			// Remove weapon if no ammo left
			chr.RemoveCard(special->Type());
		}
		// Show/hide ammo display
		M2G_PROXY.set_ammo_display_state(dynamic_cast<const m2::FastCharacter&>(chr).GetFirstCard(CARD_CATEGORY_SPECIAL_RANGED_WEAPON));
	};
	phy.onCollision = [](MAYBE m2::Physique& me, m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		if (auto* other_char = other.Owner().TryGetCharacter(); other_char && 10.0f < m2::VecF{me.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->GetLinearVelocity()}.GetLength()) {
			other_char->ExecuteInteraction(std::make_unique<m2g::Proxy::StunDuration>(2.0f));
		}
	};
	chr.onMessage = [](m2::Character& self, MAYBE m2::Character* other, const std::unique_ptr<const m2::Proxy::InterCharacterMessage>& data) -> std::unique_ptr<const m2::Proxy::InterCharacterMessage> {
		if (const auto* hitDamage = dynamic_cast<const m2g::Proxy::HitDamage*>(data.get())) {
			// Get hit by an enemy
			self.SubtractVariable(m2g::pb::RESOURCE_HP, hitDamage->hp, 0.0f);
		} else if (const auto* receivedCard = dynamic_cast<const m2g::Proxy::Card*>(data.get())) {
			const auto& card = M2_GAME.GetCard(receivedCard->type);
			// If the card in a consumable
			if (const auto consumableIt = M2G_PROXY.CONSUMABLE_BENEFITS.find(card.Type()); consumableIt != M2G_PROXY.CONSUMABLE_BENEFITS.end()) {
				// Gain the benefits
				if (consumableIt->second.first == RESOURCE_HP) {
					const auto current = self.GetVariable(consumableIt->second.first).GetFOrZero();
					self.SetVariable(consumableIt->second.first, std::min(current + consumableIt->second.second, 1.0f));
				}
			} else {
				// Player can hold only one special weapon of certain type, get rid of the previous one
				constexpr std::array<CardCategory, 2> special_categories = {CARD_CATEGORY_SPECIAL_RANGED_WEAPON, CARD_CATEGORY_SPECIAL_MELEE_WEAPON};
				constexpr std::array<VariableType, 2> special_ammo_type = {RESOURCE_SPECIAL_RANGED_WEAPON_AMMO, NO_VARIABLE};
				for (size_t i = 0; i < special_categories.size(); ++i) {
					if (auto sp = special_categories[i]; sp == card.Category()) {
						if (self.HasCard(sp)) {
							self.RemoveCard(*self.GetFirstCardType(sp)); // Remove weapon
							self.ClearVariable(special_ammo_type[i]); // Also remove any ammo
						}
						break;
					}
				}
				// Add card
				self.AddCard(receivedCard->type);
				if (const auto ammo = card.GetConstant(m2g::pb::CONSTANT_AMMO_GAIN)) {
					self.SetVariable(m2g::pb::RESOURCE_SPECIAL_RANGED_WEAPON_AMMO, ammo.GetIntOrZero());
				}
			}
		}
		return {};
	};
	gfx.preDraw = [&](MAYBE m2::Graphic& gfx, const m2::Stopwatch::Duration& delta) {
		impl.animation_fsm.time(m2::ToDurationF(delta));
	};
	gfx.onDraw = [&](m2::Graphic& gfx) {
		// Draw the sprite itself
		m2::Graphic::DefaultDrawCallback(gfx);
		// Draw the HP addon
		DrawAddons(gfx, chr.GetVariable(RESOURCE_HP).GetFOrZero());
	};

	M2_LEVEL.playerId = M2_LEVEL.objects.GetId(&obj);
	return {};
}
