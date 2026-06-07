#include <m2/Group.h>
#include <m2/M2.h>
#include <m2/Log.h>
#include <m2/Object.h>
#include <m2/game/CharacterMovement.h>
#include <rpg/Data.h>
#include <rpg/Detail.h>
#include <rpg/Objects.h>
#include <rpg/object/Enemy.h>
#include <deque>
#include <m2/thirdparty/physics/ColliderCategory.h>
#include "m2/Game.h"
#include "m2/game/Pathfinder.h"
#include <rpg/Graphic.h>
#include "rpg/group/CardGroup.h"
#include <m2/component/Character.h>

using namespace rpg;
using namespace m2g;
using namespace m2g::pb;

m2::Object& EnemyCharacter::GetOwner() {
	return M2_LEVEL.objects[GetOwnerId()];
}

void EnemyCharacter::OnUpdate(const m2::Stopwatch::Duration delta) {
	m2::UnsafeSubtractVariable(*this, RESOURCE_STUN_TTL, std::chrono::duration_cast<std::chrono::duration<float>>(delta).count(), 0.0f);
	m2::UnsafeSubtractVariable(*this, RESOURCE_DAMAGE_EFFECT_TTL, std::chrono::duration_cast<std::chrono::duration<float>>(delta).count(), 0.0f);
	m2::UnsafeAddVariable(*this, RESOURCE_RANGED_ENERGY, std::chrono::duration_cast<std::chrono::duration<float>>(delta).count());
	m2::UnsafeAddVariable(*this, RESOURCE_MELEE_ENERGY, std::chrono::duration_cast<std::chrono::duration<float>>(delta).count());
}
void EnemyCharacter::OnMessage(const m2::Interaction data) {
	if (const auto* hitDamage = dynamic_cast<const Proxy::HitDamage*>(data.get())) {
		// Deduct HP
		UnsafeSubtractVariable(*this, RESOURCE_HP, hitDamage->hp, 0.0f);
		// Apply mask effect
		UnsafeSetVariable(m2g::pb::RESOURCE_DAMAGE_EFFECT_TTL, 0.15f);
		// Play audio effect if not already doing so
		if (GetOwner().GetSoundId() == 0) {
			// Add sound emitter
			auto& sound_emitter = GetOwner().AddSoundEmitter();
			sound_emitter.update = [&](m2::SoundEmitter& se, const m2::Stopwatch::Duration&) {
				// Play sound
				if (se.playbacks.empty()) {
					auto playback_id = M2_GAME.audio_manager->Play(&M2_GAME.songs[m2g::pb::SONG_DAMAGE_TO_ENEMY], m2::AudioManager::PlayPolicy::ONCE, 0.10f);
					se.playbacks.emplace_back(playback_id);
				} else {
					// Playback finished, destroy self
					M2_LEVEL.deferredActions.push(m2::CreateSoundEmitterDeleter(GetOwner().GetId()));
				}
			};
		}
		// Check if we died
		if (not GetVariable(RESOURCE_HP)) {
			// Drop card
			auto drop_position = GetOwner().GetPhysique().position;
			if (m2::Group* group = GetOwner().TryGetGroup()) {
				// Check if the object belongs to card group
				auto* card_group = dynamic_cast<CardGroup*>(group);
				if (card_group) {
					auto optional_card = card_group->pop_card();
					if (optional_card) {
						M2_DEFER([=]() {
							create_dropped_card(*m2::CreateObject(), drop_position, *optional_card);
						});
					}
				}
			}
			// Decrement enemy counter
			M2G_PROXY.alive_enemy_count--;
			// Delete self
			LOG_INFO("Enemy died");
			M2_DEFER(m2::CreateObjectDeleter(GetOwnerId()));
			// Create corpse
			if (GetOwner().GetType() == ObjectType::SKELETON) {
				M2_DEFER([pos = drop_position]() {
					create_corpse(*m2::CreateObject(), pos, m2g::pb::SKELETON_CORPSE);
				});
			} else if (GetOwner().GetType() == ObjectType::CUTEOPUS) {
				M2_DEFER([pos = drop_position]() {
					create_corpse(*m2::CreateObject(), pos, m2g::pb::CUTEOPUS_CORPSE);
				});
			}
		} else {
			// Else, notify AI
			std::visit(m2::overloaded {
					[](ChaserFsm& v) { v.signal(ChaserFsmSignal{ChaserFsmSignal::Type::GOT_HIT}); },
					[](MAYBE auto& v) { }
			}, dynamic_cast<Enemy*>(std::get<std::unique_ptr<m2::HeapObjectImpl>>(GetOwner().impl).get())->ai_fsm);
		}
	} else if (const auto* stunDuration = dynamic_cast<const Proxy::StunDuration*>(data.get())) {
		UnsafeSetVariable(m2g::pb::RESOURCE_STUN_TTL, stunDuration->seconds);
	}
}

Enemy::Enemy(m2::Object& obj, EnemyCharacter& chr_, const pb::Enemy* enemy) : HeapObjectImpl(), chr(chr_), animation_fsm(enemy->animation_type(), obj.GetGraphicId()) {
	switch (enemy->ai().variant_case()) {
		case pb::Ai::kChaser:
			ai_fsm = ChaserFsm{&obj, &enemy->ai()};
			break;
		case pb::Ai::kHitNRun:
		case pb::Ai::kKeepDistance:
			ai_fsm = EscaperFsm{&obj, &enemy->ai()};
			break;
		case pb::Ai::kPatrol:
		default:
			throw M2_ERROR("Not yet implemented");
	}
}

m2::void_expected Enemy::init(m2::Object& obj, const m2::VecF& position) {
	const auto main_sprite_type = *M2_GAME.GetMainSpriteOfObject(obj.GetType());
	const auto& mainSprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(main_sprite_type));
	auto& gfx = obj.AddGraphic(m2g::pb::UprightGraphicsLayer::UPRIGHT_GRAPHICS_DEFAULT_LAYER, main_sprite_type);
	gfx.position = position;

	auto& phy = obj.AddPhysique();
	phy.position = position;
	m2::thirdparty::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::thirdparty::physics::RigidBodyType::DYNAMIC,
		.fixtures = {
			m2::thirdparty::physics::FixtureDefinition{
				.shape = m2::thirdparty::physics::CircleShape::FromSpriteCircleFixture(mainSprite.OriginalPb().regular().fixtures(0).circle(), mainSprite.Ppm()),
				.colliderFilter = m2::thirdparty::physics::gColliderCategoryToParams[m2::I(m2::thirdparty::physics::ColliderCategory::COLLIDER_CATEGORY_BACKGROUND_HOSTILE_OBJECT)]
			},
			m2::thirdparty::physics::FixtureDefinition{
				.shape = m2::thirdparty::physics::CircleShape::FromSpriteCircleFixture(mainSprite.OriginalPb().regular().fixtures(1).circle(), mainSprite.Ppm()),
				.colliderFilter = m2::thirdparty::physics::gColliderCategoryToParams[m2::I(m2::thirdparty::physics::ColliderCategory::COLLIDER_CATEGORY_FOREGROUND_HOSTILE_OBJECT)]
			}
		},
		.linearDamping = 5.0f,
		.fixedRotation = true,
		.mass = 20.0f, // Enemy mass is lower than player, so that player can push the enemies
		.allowSleeping = true,
		.initiallyAwake = false,
		.isBullet = false
	};
	phy.body = m2::thirdparty::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), position, {});

	auto& chr = M2_LEVEL.AddCharacterToObject<m2g::ProxyEx::EnemyCharacterStorageIndex>(obj, obj.GetId());
	chr.UnsafeAddCard(m2g::pb::CARD_REUSABLE_GUN);
	chr.UnsafeAddCard(m2g::pb::CARD_REUSABLE_ENEMY_SWORD);
	chr.UnsafeSetVariable(m2g::pb::RESOURCE_HP, 1.0f);

    obj.impl = std::make_unique<Enemy>(obj, chr, M2G_PROXY.get_enemy(obj.GetType()));
	auto& impl = dynamic_cast<Enemy&>(*std::get<std::unique_ptr<HeapObjectImpl>>(obj.impl));

	// Increment enemy counter
	M2G_PROXY.alive_enemy_count++;

	phy.preStep = [&](MAYBE m2::Physique& phy, const m2::Stopwatch::Duration& delta) {
		std::visit(m2::overloaded {
			[](MAYBE std::monostate& v) {},
			[&delta](auto& v) { v.time(m2::ToDurationF(delta)); }
		}, impl.ai_fsm);
		std::visit(m2::overloaded {
			[](ChaserFsm& v) { v.signal(ChaserFsmSignal{ChaserFsmSignal::Type::PHY_STEP}); },
			[](EscaperFsm& v) { v.signal(EscaperFsmSignal{}); },
			[](MAYBE auto& v) { }
		}, impl.ai_fsm);
	};
	phy.postStep = [&](MAYBE m2::Physique& phy, const m2::Stopwatch::Duration&) {
		m2::VecF velocity = m2::VecF{std::get<m2::Physique::Body>(phy.body).GetLinearVelocity()};
		if (velocity.IsNear(m2::VecF{}, 0.1f)) {
			impl.animation_fsm.signal(m2::AnimationFsmSignal{m2g::pb::ANIMATION_STATE_IDLE});
		}
	};
	gfx.preDraw = [&](m2::Graphic&, const m2::Stopwatch::Duration& delta) {
		using namespace m2::pb;
		impl.animation_fsm.time(m2::ToDurationF(delta));
		if (chr.GetVariable(RESOURCE_DAMAGE_EFFECT_TTL)) {
			impl.animation_fsm.signal(m2::AnimationFsmSignal{ANIMATION_STATE_HURT});
		} else {
			impl.animation_fsm.signal(m2::AnimationFsmSignal{ANIMATION_STATE_IDLE});
		}
	};
	gfx.onDraw = [&](m2::Graphic& gfx) {
		// Draw the sprite itself
		m2::Graphic::DefaultDrawCallback(gfx);
		// Draw the HP addon
		DrawAddons(gfx, chr.GetVariable(RESOURCE_HP).GetFOrZero());
#ifdef DEBUG
		std::visit(m2::overloaded {
				[](ChaserFsm& v) { m2::Pathfinder::draw_path(v.reverse_path(), SDL_Color{127, 127, 255, 255}); },
				[](MAYBE auto& v) { }
		}, impl.ai_fsm);
#endif
	};

	return {};
}

void rpg::Enemy::move_towards(m2::Object& obj, m2::VecF direction, float force) {
	auto& chr = dynamic_cast<Enemy*>(std::get<std::unique_ptr<m2::HeapObjectImpl>>(obj.impl).get())->chr;
	// If not stunned
	if (not chr.GetVariable(m2g::pb::RESOURCE_STUN_TTL)) {
		direction = direction.Normalize();
		// Walk animation
		auto char_move_dir = m2::to_character_movement_direction(direction);
		auto anim_state_type = rpg::detail::to_animation_state_type(char_move_dir);
		dynamic_cast<Enemy&>(*std::get<std::unique_ptr<HeapObjectImpl>>(obj.impl)).animation_fsm.signal(m2::AnimationFsmSignal{anim_state_type});
		// Apply force
		m2::VecF force_direction = direction * (m2::ToDurationF(m2::TIME_BETWEEN_PHYSICS_SIMULATIONS) * force);
		std::get<m2::Physique::Body>(obj.GetPhysique().body).ApplyForceToCenter(force_direction);
	}
}

void rpg::Enemy::attack_if_close(m2::Object& obj, const pb::Ai& ai) {
	const auto& selfPosition = obj.GetPhysique().position;
	auto& chr = dynamic_cast<Enemy*>(std::get<std::unique_ptr<m2::HeapObjectImpl>>(obj.impl).get())->chr;
	// If not stunned
	if (not chr.GetVariable(m2g::pb::RESOURCE_STUN_TTL)) {
		// Attack if player is close
		if (selfPosition.IsNear(M2_PLAYER.GetPhysique().position, ai.attack_distance())) {
			// Based on what the capability is
			auto capability = ai.capabilities(0);
			switch (capability) {
				case pb::CAPABILITY_RANGED: {
					if (const auto* rangedWeapon = chr.GetFirstCard(CARD_CATEGORY_DEFAULT_RANGED_WEAPON);
						rangedWeapon && rangedWeapon->GetConstant(CONSTANT_RANGED_ENERGY_REQUIREMENT).GetFOrZero() <= chr.GetVariable(RESOURCE_RANGED_ENERGY).GetFOrZero()) {
						chr.ClearVariable(RESOURCE_RANGED_ENERGY);
						const auto shoot_direction = M2_PLAYER.GetPhysique().position - selfPosition;
						create_projectile(*m2::CreateObject({}, obj.GetId()), selfPosition, shoot_direction, *rangedWeapon, false);
						// Knock-back
						std::get<m2::Physique::Body>(obj.GetPhysique().body).ApplyForceToCenter(m2::VecF::CreateUnitVectorWithAngle(shoot_direction.GetAngle() + m2::PI) * 5000.0f);
					}
					break;
				}
				case pb::CAPABILITY_MELEE: {
					if (const auto* meleeWeapon = chr.GetFirstCard(CARD_CATEGORY_DEFAULT_MELEE_WEAPON);
						meleeWeapon && meleeWeapon->GetConstant(CONSTANT_MELEE_ENERGY_REQUIREMENT).GetFOrZero() <= chr.GetVariable(RESOURCE_MELEE_ENERGY).GetFOrZero()) {
						chr.ClearVariable(RESOURCE_MELEE_ENERGY);
						create_blade(*m2::CreateObject({}, obj.GetId()), selfPosition,
							M2_PLAYER.GetPhysique().position - selfPosition, *meleeWeapon, false);
					}
					break;
				}
				case pb::CAPABILITY_EXPLOSIVE:
					throw M2_ERROR("Chaser explosive weapon not implemented");
				case pb::CAPABILITY_KAMIKAZE:
					throw M2_ERROR("Chaser kamikaze not implemented");
				default:
					break;
			}
		}
	}
}
