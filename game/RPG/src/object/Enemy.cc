#include <rpg/object/Enemy.h>
#include <m2/game/CharacterMovement.h>
#include <rpg/Objects.h>
#include <m2/Object.h>
#include "m2/Game.h"
#include "rpg/group/ItemGroup.h"
#include <rpg/Detail.h>
#include <rpg/Context.h>
#include <m2/M2.h>
#include <m2/box2d/Detail.h>
#include <m2/Group.h>
#include <m2g_Interaction.pb.h>
#include <m2/Pathfinder.h>
#include <deque>

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
			throw M2ERROR("Not yet implemented");
	}
}

m2::void_expected Enemy::init(m2::Object& obj, m2g::pb::ObjectType object_type) {
	auto main_sprite_type = GAME.level_editor_object_sprites[object_type];

	auto& gfx = obj.add_graphic(GAME.get_sprite(main_sprite_type));

	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::DYNAMIC);
	bp.set_allow_sleep(true);
	bp.mutable_background_fixture()->mutable_circ()->set_radius(GAME.get_sprite(main_sprite_type).background_collider_circ_radius_m());
	bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::FOE_ON_BACKGROUND);
	bp.mutable_foreground_fixture()->mutable_circ()->set_radius(GAME.get_sprite(main_sprite_type).foreground_collider_circ_radius_m());
	bp.mutable_foreground_fixture()->set_category(m2::pb::FixtureCategory::FOE_ON_FOREGROUND);
	bp.set_mass(20.0f); // Enemy mass is lower than player, so that player can push the enemies
	bp.set_linear_damping(5.0f);
	bp.set_fixed_rotation(true);
	phy.body = m2::box2d::create_body(*LEVEL.world, obj.physique_id(), obj.position, bp);

	auto& chr = obj.add_full_character();
	chr.add_item(GAME.get_item(m2g::pb::ITEM_REUSABLE_GUN));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_REUSABLE_ENEMY_SWORD));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_AUTOMATIC_DAMAGE_EFFECT_TTL));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_AUTOMATIC_RANGED_ENERGY));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_AUTOMATIC_MELEE_ENERGY));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_AUTOMATIC_STUN_TTL));
	chr.add_resource(m2g::pb::RESOURCE_HP, 1.0f);

    obj.impl = std::make_unique<Enemy>(obj, Context::get_instance().get_enemy(object_type));
	auto& impl = dynamic_cast<Enemy&>(*obj.impl);

	// Increment enemy counter
	auto& context = Context::get_instance();
	context.alive_enemy_count++;

	phy.pre_step = [&](MAYBE m2::Physique& phy) {
		std::visit(m2::overloaded {
			[](MAYBE std::monostate& v) {},
			[](auto& v) { v.time(GAME.delta_time_s()); }
		}, impl.ai_fsm);
		std::visit(m2::overloaded {
			[](ChaserFsm& v) { v.signal(ChaserFsmSignal{ChaserFsmSignal::Type::PHY_STEP}); },
			[](EscaperFsm& v) { v.signal(EscaperFsmSignal{}); },
			[](MAYBE auto& v) { }
		}, impl.ai_fsm);
	};
	chr.get_interacted_by = [&, obj_type = object_type](m2::Character& self, MAYBE m2::Character& other, m2g::pb::InteractionType type, const m2g::pb::InteractionData& data) {
		if (type == InteractionType::HIT) {
			// Deduct HP
			self.remove_resource(RESOURCE_HP, data.hit_damage());
			// Apply mask effect
			self.set_resource(m2g::pb::RESOURCE_DAMAGE_EFFECT_TTL, 0.15f);
			// Play audio effect if not already doing so
			if (obj.sound_id() == 0) {
				// Add sound emitter
				auto& sound_emitter = obj.add_sound_emitter();
				sound_emitter.update = [&](m2::SoundEmitter& se) {
					// Play sound
					if (se.playbacks.empty()) {
						auto playback_id = GAME.audio_manager->play(&GAME.get_song(m2g::pb::SONG_DAMAGE_TO_ENEMY), m2::AudioManager::PlayPolicy::ONCE, 0.10f);
						se.playbacks.emplace_back(playback_id);
					} else {
						// Playback finished, destroy self
						LEVEL.deferred_actions.emplace_back(m2::create_sound_emitter_deleter(obj.id()));
					}
				};
			}
			// Check if we died
			if (not self.has_resource(RESOURCE_HP)) {
				// Drop item
				auto drop_position = obj.position;
				m2::Group* group = obj.group();
				if (group) {
					// Check if the object belongs to item group
					auto* item_group = dynamic_cast<ItemGroup*>(group);
					if (item_group) {
						auto optional_item = item_group->pop_item();
						if (optional_item) {
							GAME.add_deferred_action([=]() {
								create_dropped_item(m2::create_object(drop_position).first, *optional_item);
							});
						}
					}
				}
				// Decrement enemy counter
				auto& context = Context::get_instance();
				context.alive_enemy_count--;
				// Delete self
				LOG_INFO("Enemy died");
				GAME.add_deferred_action(m2::create_object_deleter(self.object_id));
				// Create corpse
				if (obj_type == ObjectType::SKELETON) {
					GAME.add_deferred_action([pos = obj.position]() {
						auto& corpse = m2::create_object(pos).first;
						create_corpse(corpse, m2g::pb::SKELETON_CORPSE);
					});
				} else if (obj_type == ObjectType::CUTEOPUS) {
					GAME.add_deferred_action([pos = obj.position]() {
						auto& corpse = m2::create_object(pos).first;
						create_corpse(corpse, m2g::pb::CUTEOPUS_CORPSE);
					});
				}
			} else {
				// Else, notify AI
				std::visit(m2::overloaded {
						[](ChaserFsm& v) { v.signal(ChaserFsmSignal{ChaserFsmSignal::Type::GOT_HIT}); },
						[](MAYBE auto& v) { }
				}, impl.ai_fsm);
			}
		} else if (type == STUN) {
			self.set_resource(m2g::pb::RESOURCE_STUN_TTL, data.stun_duration());
		}
	};
	phy.post_step = [&](MAYBE m2::Physique& phy) {
		m2::VecF velocity = m2::VecF{phy.body->GetLinearVelocity()};
		if (velocity.is_near(m2::VecF{}, 0.1f)) {
			impl.animation_fsm.signal(m2::AnimationFsmSignal{m2g::pb::ANIMATION_STATE_IDLE});
		}
	};
	gfx.pre_draw = [&](m2::Graphic& gfx) {
		using namespace m2::pb;
		impl.animation_fsm.time(GAME.delta_time_s());
		gfx.draw_addon_health_bar = chr.get_resource(RESOURCE_HP);
		gfx.draw_sprite_effect = chr.has_resource(RESOURCE_DAMAGE_EFFECT_TTL) ? SPRITE_EFFECT_MASK : NO_SPRITE_EFFECT;
	};
	phy.on_debug_draw = [&impl](m2::Physique& phy) {
		m2::Physique::default_debug_draw(phy);
		std::visit(m2::overloaded {
				[](ChaserFsm& v) { m2::Pathfinder::draw_path(v.reverse_path(), SDL_Color{127, 127, 255, 255}); },
				[](MAYBE auto& v) { }
		}, impl.ai_fsm);
	};

	return {};
}

void rpg::Enemy::move_towards(m2::Object& obj, m2::VecF direction, float force) {
	// If not stunned
	if (not obj.character().has_resource(m2g::pb::RESOURCE_STUN_TTL)) {
		direction = direction.normalize();
		// Walk animation
		auto char_move_dir = m2::to_character_movement_direction(direction);
		auto anim_state_type = rpg::detail::to_animation_state_type(char_move_dir);
		dynamic_cast<Enemy&>(*obj.impl).animation_fsm.signal(m2::AnimationFsmSignal{anim_state_type});
		// Apply force
		m2::VecF force_direction = direction * (GAME.delta_time_s() * force);
		obj.physique().body->ApplyForceToCenter(static_cast<b2Vec2>(force_direction), true);
	}
}

void rpg::Enemy::attack_if_close(m2::Object& obj, const pb::Ai& ai) {
	// If not stunned
	if (not obj.character().has_resource(m2g::pb::RESOURCE_STUN_TTL)) {
		// Attack if player is close
		if (obj.position.is_near(LEVEL.player()->position, ai.attack_distance())) {
			// Based on what the capability is
			auto capability = ai.capabilities(0);
			switch (capability) {
				case pb::CAPABILITY_RANGED: {
					auto it = obj.character().find_items(m2g::pb::ITEM_CATEGORY_DEFAULT_RANGED_WEAPON);
					if (it && obj.character().use_item(it)) {
						auto& projectile = m2::create_object(obj.position, obj.id()).first;
						auto shoot_direction = LEVEL.player()->position - obj.position;
						rpg::create_projectile(projectile, shoot_direction, *it, false);
						// Knock-back
						obj.physique().body->ApplyForceToCenter(static_cast<b2Vec2>(m2::VecF::from_angle(shoot_direction.angle_rads() + m2::PI) * 5000.0f), true);
					}
					break;
				}
				case pb::CAPABILITY_MELEE: {
					auto it = obj.character().find_items(m2g::pb::ITEM_CATEGORY_DEFAULT_MELEE_WEAPON);
					if (it && obj.character().use_item(it)) {
						auto& melee = m2::create_object(obj.position, obj.id()).first;
						rpg::create_blade(melee, LEVEL.player()->position - obj.position, *it, false);
					}
					break;
				}
				case pb::CAPABILITY_EXPLOSIVE:
					throw M2ERROR("Chaser explosive weapon not implemented");
				case pb::CAPABILITY_KAMIKAZE:
					throw M2ERROR("Chaser kamikaze not implemented");
				default:
					break;
			}
		}
	}
}
