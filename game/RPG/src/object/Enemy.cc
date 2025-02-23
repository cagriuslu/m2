#include <m2/Group.h>
#include <m2/M2.h>
#include <m2/Log.h>
#include <m2/Object.h>
#include <m2/box2d/Detail.h>
#include <m2/game/CharacterMovement.h>
#include <m2g_Interaction.pb.h>
#include <rpg/Data.h>
#include <rpg/Detail.h>
#include <rpg/Objects.h>
#include <rpg/object/Enemy.h>

#include <deque>

#include "m2/Game.h"
#include "m2/game/Pathfinder.h"
#include <rpg/Graphic.h>
#include "rpg/group/ItemGroup.h"

using namespace rpg;
using namespace m2g;
using namespace m2g::pb;

Enemy::Enemy(m2::Object& obj, const pb::Enemy* enemy) : animation_fsm(enemy->animation_type(), obj.graphic_id()) {
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

m2::void_expected Enemy::init(m2::Object& obj) {
	const auto main_sprite_type = M2_GAME.object_main_sprites[obj.object_type()];
	const auto& mainSprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(main_sprite_type));
	auto& gfx = obj.add_graphic(main_sprite_type);

	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::DYNAMIC);
	bp.set_allow_sleep(true);
	bp.mutable_background_fixture()->mutable_circ()->set_radius(mainSprite.BackgroundColliderCircRadiusM());
	bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::FOE_ON_BACKGROUND);
	bp.mutable_foreground_fixture()->mutable_circ()->set_radius(mainSprite.ForegroundColliderCircRadiusM());
	bp.mutable_foreground_fixture()->set_category(m2::pb::FixtureCategory::FOE_ON_FOREGROUND);
	bp.set_mass(20.0f); // Enemy mass is lower than player, so that player can push the enemies
	bp.set_linear_damping(5.0f);
	bp.set_fixed_rotation(true);
	phy.body = m2::box2d::CreateBody(*M2_LEVEL.world, obj.physique_id(), obj.position, bp);

	auto& chr = obj.add_full_character();
	chr.AddNamedItem(M2_GAME.GetNamedItem(m2g::pb::ITEM_REUSABLE_GUN));
	chr.AddNamedItem(M2_GAME.GetNamedItem(m2g::pb::ITEM_REUSABLE_ENEMY_SWORD));
	chr.AddNamedItem(M2_GAME.GetNamedItem(m2g::pb::ITEM_AUTOMATIC_DAMAGE_EFFECT_TTL));
	chr.AddNamedItem(M2_GAME.GetNamedItem(m2g::pb::ITEM_AUTOMATIC_RANGED_ENERGY));
	chr.AddNamedItem(M2_GAME.GetNamedItem(m2g::pb::ITEM_AUTOMATIC_MELEE_ENERGY));
	chr.AddNamedItem(M2_GAME.GetNamedItem(m2g::pb::ITEM_AUTOMATIC_STUN_TTL));
	chr.AddResource(m2g::pb::RESOURCE_HP, 1.0f);

    obj.impl = std::make_unique<Enemy>(obj, M2G_PROXY.get_enemy(obj.object_type()));
	auto& impl = dynamic_cast<Enemy&>(*obj.impl);

	// Increment enemy counter
	M2G_PROXY.alive_enemy_count++;

	phy.preStep = [&](MAYBE m2::Physique& phy) {
		std::visit(m2::overloaded {
			[](MAYBE std::monostate& v) {},
			[](auto& v) { v.time(M2_GAME.DeltaTimeS()); }
		}, impl.ai_fsm);
		std::visit(m2::overloaded {
			[](ChaserFsm& v) { v.signal(ChaserFsmSignal{ChaserFsmSignal::Type::PHY_STEP}); },
			[](EscaperFsm& v) { v.signal(EscaperFsmSignal{}); },
			[](MAYBE auto& v) { }
		}, impl.ai_fsm);
	};
	chr.on_interaction = [&, obj_type = obj.object_type()](m2::Character& self, MAYBE m2::Character* other, const InteractionData& data) -> std::optional<m2g::pb::InteractionData> {
		if (data.has_hit_damage()) {
			// Deduct HP
			self.RemoveResource(RESOURCE_HP, data.hit_damage());
			// Apply mask effect
			self.SetResource(m2g::pb::RESOURCE_DAMAGE_EFFECT_TTL, 0.15f);
			// Play audio effect if not already doing so
			if (obj.sound_id() == 0) {
				// Add sound emitter
				auto& sound_emitter = obj.add_sound_emitter();
				sound_emitter.update = [&](m2::SoundEmitter& se) {
					// Play sound
					if (se.playbacks.empty()) {
						auto playback_id = M2_GAME.audio_manager->Play(&M2_GAME.songs[m2g::pb::SONG_DAMAGE_TO_ENEMY], m2::AudioManager::PlayPolicy::ONCE, 0.10f);
						se.playbacks.emplace_back(playback_id);
					} else {
						// Playback finished, destroy self
						M2_LEVEL.deferredActions.push(m2::create_sound_emitter_deleter(obj.id()));
					}
				};
			}
			// Check if we died
			if (not self.HasResource(RESOURCE_HP)) {
				// Drop item
				auto drop_position = obj.position;
				if (m2::Group* group = obj.get_group()) {
					// Check if the object belongs to item group
					auto* item_group = dynamic_cast<ItemGroup*>(group);
					if (item_group) {
						auto optional_item = item_group->pop_item();
						if (optional_item) {
							M2_DEFER([=]() {
								create_dropped_item(*m2::create_object(drop_position), *optional_item);
							});
						}
					}
				}
				// Decrement enemy counter
				M2G_PROXY.alive_enemy_count--;
				// Delete self
				LOG_INFO("Enemy died");
				M2_DEFER(m2::create_object_deleter(self.owner_id()));
				// Create corpse
				if (obj_type == ObjectType::SKELETON) {
					M2_DEFER([pos = obj.position]() {
						create_corpse(*m2::create_object(pos), m2g::pb::SKELETON_CORPSE);
					});
				} else if (obj_type == ObjectType::CUTEOPUS) {
					M2_DEFER([pos = obj.position]() {
						create_corpse(*m2::create_object(pos), m2g::pb::CUTEOPUS_CORPSE);
					});
				}
			} else {
				// Else, notify AI
				std::visit(m2::overloaded {
						[](ChaserFsm& v) { v.signal(ChaserFsmSignal{ChaserFsmSignal::Type::GOT_HIT}); },
						[](MAYBE auto& v) { }
				}, impl.ai_fsm);
			}
		} else if (data.has_stun_duration()) {
			self.SetResource(m2g::pb::RESOURCE_STUN_TTL, data.stun_duration());
		}
		return std::nullopt;
	};
	phy.postStep = [&](MAYBE m2::Physique& phy) {
		m2::VecF velocity = m2::VecF{phy.body->GetLinearVelocity()};
		if (velocity.is_near(m2::VecF{}, 0.1f)) {
			impl.animation_fsm.signal(m2::AnimationFsmSignal{m2g::pb::ANIMATION_STATE_IDLE});
		}
	};
	gfx.preDraw = [&](m2::Graphic& gfx) {
		using namespace m2::pb;
		impl.animation_fsm.time(M2_GAME.DeltaTimeS());
		if (chr.HasResource(RESOURCE_DAMAGE_EFFECT_TTL)) {
			gfx.variantDrawOrder[0] = SPRITE_EFFECT_MASK;
		} else {
			gfx.variantDrawOrder[0] = std::nullopt;
		}
	};
	gfx.onDraw = [&](m2::Graphic& gfx) {
		// Draw the sprite itself
		m2::Graphic::DefaultDrawCallback(gfx);
		// Draw the HP addon
		DrawAddons(gfx, chr.GetResource(RESOURCE_HP));
	};
	phy.onDebugDraw = [&impl](m2::Physique& phy) {
		m2::Physique::DefaultDebugDraw(phy);
		std::visit(m2::overloaded {
				[](ChaserFsm& v) { m2::Pathfinder::draw_path(v.reverse_path(), SDL_Color{127, 127, 255, 255}); },
				[](MAYBE auto& v) { }
		}, impl.ai_fsm);
	};

	return {};
}

void rpg::Enemy::move_towards(m2::Object& obj, m2::VecF direction, float force) {
	// If not stunned
	if (not obj.character().HasResource(m2g::pb::RESOURCE_STUN_TTL)) {
		direction = direction.normalize();
		// Walk animation
		auto char_move_dir = m2::to_character_movement_direction(direction);
		auto anim_state_type = rpg::detail::to_animation_state_type(char_move_dir);
		dynamic_cast<Enemy&>(*obj.impl).animation_fsm.signal(m2::AnimationFsmSignal{anim_state_type});
		// Apply force
		m2::VecF force_direction = direction * (M2_GAME.DeltaTimeS() * force);
		obj.physique().body->ApplyForceToCenter(static_cast<b2Vec2>(force_direction), true);
	}
}

void rpg::Enemy::attack_if_close(m2::Object& obj, const pb::Ai& ai) {
	// If not stunned
	if (not obj.character().HasResource(m2g::pb::RESOURCE_STUN_TTL)) {
		// Attack if player is close
		if (obj.position.is_near(M2_PLAYER.position, ai.attack_distance())) {
			// Based on what the capability is
			auto capability = ai.capabilities(0);
			switch (capability) {
				case pb::CAPABILITY_RANGED: {
					auto it = obj.character().FindItems(m2g::pb::ITEM_CATEGORY_DEFAULT_RANGED_WEAPON);
					if (it && obj.character().UseItem(it)) {
						auto shoot_direction = M2_PLAYER.position - obj.position;
						rpg::create_projectile(*m2::create_object(obj.position, {}, obj.id()),
							shoot_direction, *it, false);
						// Knock-back
						obj.physique().body->ApplyForceToCenter(static_cast<b2Vec2>(m2::VecF::from_angle(shoot_direction.angle_rads() + m2::PI) * 5000.0f), true);
					}
					break;
				}
				case pb::CAPABILITY_MELEE: {
					auto it = obj.character().FindItems(m2g::pb::ITEM_CATEGORY_DEFAULT_MELEE_WEAPON);
					if (it && obj.character().UseItem(it)) {
						rpg::create_blade(*m2::create_object(obj.position, {}, obj.id()),
							M2_PLAYER.position - obj.position, *it, false);
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
