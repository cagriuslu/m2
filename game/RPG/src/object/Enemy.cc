#include <rpg/object/Enemy.h>
#include <rpg/object/DroppedItem.h>
#include <m2/Object.h>
#include "m2/Game.hh"
#include "rpg/group/ItemGroup.h"
#include <m2/M2.h>
#include <m2g/component/Defense.h>
#include <m2/box2d/Utils.h>
#include <m2g/SpriteBlueprint.h>
#include <m2/Group.h>
#include <deque>

using namespace obj;

obj::Enemy::Enemy(m2::Object& obj, const chr::CharacterBlueprint* blueprint) : character_state(blueprint),
	animation_fsm(blueprint->animation_type, obj.graphic_id()), fsm_variant(
		std::visit(m2::overloaded {
			[&](MAYBE const ai::type::ChaseBlueprint& v) -> FSMVariant { return m2::Fsm<rpg::ChaserFsmBase>{&obj, blueprint->aiBlueprint}; },
			[&](MAYBE const ai::type::HitNRunBlueprint& v) -> FSMVariant { return m2::Fsm<rpg::ChaserFsmBase>{&obj, blueprint->aiBlueprint}; }, // TODO implement other FSMs
			[&](MAYBE const ai::type::KeepDistanceBlueprint& v) -> FSMVariant { return m2::Fsm<rpg::ChaserFsmBase>{&obj, blueprint->aiBlueprint}; },
			[&](MAYBE const ai::type::PatrolBlueprint& v) -> FSMVariant { return m2::Fsm<rpg::ChaserFsmBase>{&obj, blueprint->aiBlueprint}; }
		}, blueprint->aiBlueprint->variant)), on_hit_color_mod_ttl(0) {}

void Enemy::stun() {
	character_state.stun();
}

m2::VoidValue Enemy::init(m2::Object& obj, const chr::CharacterBlueprint* blueprint) {
	auto& gfx = obj.add_graphic(GAME.sprites[blueprint->main_sprite]);

	auto& monitor = obj.add_monitor();

	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::DYNAMIC);
	bp.mutable_circ()->set_radius(GAME.sprites[blueprint->main_sprite].collider_circ_radius_m());
	bp.set_allow_sleep(true);
	bp.set_is_bullet(false);
	bp.set_is_sensor(false);
	bp.set_category(m2::pb::BodyCategory::FOE);
	bp.set_mass(blueprint->mass_kg);
	bp.set_linear_damping(blueprint->linear_damping);
	bp.set_fixed_rotation(true);
	phy.body = m2::box2d::create_body(*GAME.world, obj.physique_id(), obj.position, bp);

	auto& def = obj.add_defense();
	def.hp = 100;
	def.maxHp = 100;

    obj.impl = std::make_unique<obj::Enemy>(obj, blueprint);

	monitor.pre_phy = [&](MAYBE m2::comp::Monitor& mon) {
		auto* impl = dynamic_cast<Enemy*>(obj.impl.get());
		impl->character_state.process_time(GAME.deltaTime_s);
		std::visit([](auto& v) { v.time(GAME.deltaTime_s); }, impl->fsm_variant);
		std::visit([](auto& v) { v.signal(rpg::AI_FSM_SIGNAL_PREPHY); }, impl->fsm_variant);
	};

	monitor.post_phy = [&](MAYBE m2::comp::Monitor& mon) {
		auto* data = dynamic_cast<Enemy*>(obj.impl.get());
		// We must call time before other signals
		data->animation_fsm.time(GAME.deltaTicks_ms / 1000.0f);
		m2::Vec2f velocity = m2::Vec2f{phy.body->GetLinearVelocity() };
		if (fabsf(velocity.x) < 0.5000f && fabsf(velocity.y) < 0.5000f) {
			data->animation_fsm.signal(m2g::pb::ANIMATION_STATE_IDLE);
		} else if (fabsf(velocity.x) < fabsf(velocity.y)) {
			if (0 < velocity.y) {
				data->animation_fsm.signal(m2g::pb::ANIMATION_STATE_WALKDOWN);
			} else {
				data->animation_fsm.signal(m2g::pb::ANIMATION_STATE_WALKUP);
			}
		} else {
			if (0 < velocity.x) {
				data->animation_fsm.signal(m2g::pb::ANIMATION_STATE_WALKRIGHT);
			} else {
				data->animation_fsm.signal(m2g::pb::ANIMATION_STATE_WALKLEFT);
			}
		}
	};

	gfx.on_draw = [&](m2::comp::Graphic& gfx) {
		auto* data = dynamic_cast<Enemy*>(obj.impl.get());
		if (0.0f < data->on_hit_color_mod_ttl) {
			SDL_Texture *defaultTexture = gfx.texture;
			gfx.texture = GAME.sdlTextureMask;
			m2::comp::Graphic::default_draw(gfx);
			gfx.texture = defaultTexture;
			data->on_hit_color_mod_ttl -= GAME.deltaTicks_ms / 1000.0f;
		} else {
			m2::comp::Graphic::default_draw(gfx);
		}
		m2::comp::Graphic::default_draw_healthbar(gfx, (float) def.hp / def.maxHp);
	};

	def.on_hit = [&](MAYBE m2g::comp::Defense& def) {
		auto* data = dynamic_cast<Enemy*>(obj.impl.get());
		data->on_hit_color_mod_ttl = 0.10f;
	};

	def.on_death = [&](m2g::comp::Defense& def) {
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
		// Delete self
		GAME.add_deferred_action(m2::create_object_deleter(def.object_id));
	};

	return {};
}
