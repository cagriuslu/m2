#include <rpg/object/Enemy.h>
#include <rpg/object/DroppedItem.h>
#include <m2/Object.h>
#include "m2/Game.h"
#include "rpg/group/ItemGroup.h"
#include <rpg/Context.h>
#include <m2/M2.h>
#include <m2/box2d/Detail.h>
#include <m2/Group.h>
#include <InteractionType.pb.h>
#include <deque>

using namespace obj;
using namespace m2g;
using namespace m2g::pb;

obj::Enemy::Enemy(m2::Object& obj, const rpg::pb::Enemy* enemy) : animation_fsm(enemy->animation_type(), obj.graphic_id()) {
	switch (enemy->ai().variant_case()) {
		case rpg::pb::Ai::kChaser:
			ai_fsm = rpg::ChaserFsm{&obj, &enemy->ai()};
			break;
		case rpg::pb::Ai::kHitNRun:
		case rpg::pb::Ai::kKeepDistance:
		case rpg::pb::Ai::kPatrol:
		default:
			throw M2ERROR("Not yet implemented");
	}
}

m2::VoidValue Enemy::init(m2::Object& obj, m2g::pb::ObjectType object_type) {
	auto main_sprite_type = GAME.level_editor_object_sprites[object_type];

	auto& gfx = obj.add_graphic(GAME.get_sprite(main_sprite_type));

	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::DYNAMIC);
	bp.set_allow_sleep(true);
	bp.set_is_bullet(false);
	bp.mutable_background_fixture()->mutable_circ()->set_radius(GAME.get_sprite(main_sprite_type).background_collider_circ_radius_m());
	bp.mutable_background_fixture()->set_is_sensor(false);
	bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::FOE_ON_BACKGROUND);
	bp.mutable_foreground_fixture()->mutable_circ()->set_radius(GAME.get_sprite(main_sprite_type).foreground_collider_circ_radius_m());
	bp.mutable_foreground_fixture()->set_is_sensor(false);
	bp.mutable_foreground_fixture()->set_category(m2::pb::FixtureCategory::FOE_ON_FOREGROUND);
	bp.set_mass(10.0f);
	bp.set_linear_damping(10.0f);
	bp.set_fixed_rotation(true);
	phy.body = m2::box2d::create_body(*LEVEL.world, obj.physique_id(), obj.position, bp);

	auto& chr = obj.add_full_character();
	chr.add_item(GAME.get_item(m2g::pb::ITEM_REUSABLE_SWORD));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_AUTOMATIC_MELEE_ENERGY));
	chr.add_resource(m2g::pb::RESOURCE_HP, 1.0f);

    obj.impl = std::make_unique<obj::Enemy>(obj, rpg::Context::get_instance().get_enemy(object_type));

	// Increment enemy counter
	auto& context = rpg::Context::get_instance();
	context.alive_enemy_count++;

	phy.pre_step = [&obj](MAYBE m2::Physique& phy) {
		auto* impl = dynamic_cast<Enemy*>(obj.impl.get());
		std::visit(m2::overloaded {
			[](MAYBE std::monostate& v) {},
			[](auto& v) { v.time(GAME.deltaTime_s); }
		}, impl->ai_fsm);
		std::visit(m2::overloaded {
			[](rpg::ChaserFsm& v) { v.signal(rpg::ChaserFsmSignal{}); },
			[](MAYBE auto& v) { }
		}, impl->ai_fsm);
	};
	chr.interact = [&](m2::Character& self, MAYBE m2::Character& other, m2g::pb::InteractionType interaction_type) {
		// Check if we got hit
		if (interaction_type == InteractionType::GET_COLLIDED_BY) {
			// Play audio effect if not already doing so
			if (obj.sound_id() == 0) {
				// Add sound emitter
				auto& sound_emitter = obj.add_sound_emitter();
				sound_emitter.on_update = [&](m2::SoundEmitter& se) {
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
			// Apply mask effect
			self.set_resource(m2g::pb::RESOURCE_DAMAGE_EFFECT_TTL, 0.15f);
			gfx.draw_sprite_effect = m2::pb::SPRITE_EFFECT_MASK;
			if (not self.has_item(m2g::pb::ITEM_AUTOMATIC_DAMAGE_EFFECT_TTL)) {
				self.add_item(GAME.get_item(m2g::pb::ITEM_AUTOMATIC_DAMAGE_EFFECT_TTL));
			}
			// Check if we died
			if (not self.has_resource(RESOURCE_HP)) {
				// Drop item
				auto drop_position = obj.position;
				m2::Group* group = obj.group();
				if (group) {
					// Check if the object belongs to item group
					auto* item_group = dynamic_cast<rpg::ItemGroup*>(group);
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
				auto& context = rpg::Context::get_instance();
				context.alive_enemy_count--;
				// Delete self
				GAME.add_deferred_action(m2::create_object_deleter(self.object_id));
			}
		} else if (interaction_type == InteractionType::GET_STUNNED_BY) {
			self.set_resource(m2g::pb::RESOURCE_STUN_TTL, 2.0f);
			if (not self.has_item(m2g::pb::ITEM_AUTOMATIC_STUN_TTL)) {
				self.add_item(GAME.get_item(m2g::pb::ITEM_AUTOMATIC_STUN_TTL));
			}
		}
	};
	phy.post_step = [&obj](m2::Physique& phy) {
		auto* data = dynamic_cast<Enemy*>(obj.impl.get());
		// We must call time before other signals
		data->animation_fsm.time(GAME.deltaTime_s);
		m2::Vec2f velocity = m2::Vec2f{phy.body->GetLinearVelocity() };
		if (fabsf(velocity.x) < 0.5000f && fabsf(velocity.y) < 0.5000f) {
			data->animation_fsm.signal(m2::AnimationFsmSignal{m2g::pb::ANIMATION_STATE_IDLE});
		} else if (fabsf(velocity.x) < fabsf(velocity.y)) {
			if (0 < velocity.y) {
				data->animation_fsm.signal(m2::AnimationFsmSignal{m2g::pb::ANIMATION_STATE_WALKDOWN});
			} else {
				data->animation_fsm.signal(m2::AnimationFsmSignal{m2g::pb::ANIMATION_STATE_WALKUP});
			}
		} else {
			if (0 < velocity.x) {
				data->animation_fsm.signal(m2::AnimationFsmSignal{m2g::pb::ANIMATION_STATE_WALKRIGHT});
			} else {
				data->animation_fsm.signal(m2::AnimationFsmSignal{m2g::pb::ANIMATION_STATE_WALKLEFT});
			}
		}
	};
	gfx.pre_draw = [&](m2::Graphic& gfx) {
		gfx.draw_effect_health_bar = chr.get_resource(RESOURCE_HP);

		if (not chr.has_resource(m2g::pb::RESOURCE_DAMAGE_EFFECT_TTL)) {
			gfx.draw_sprite_effect = m2::pb::NO_SPRITE_EFFECT;
		}
	};

	return {};
}
