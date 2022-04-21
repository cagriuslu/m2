#include <rpg/object/Enemy.h>
#include <m2/Object.h>
#include "m2/Game.hh"
#include "m2/Def.h"
#include <m2g/component/Defense.h>
#include <m2g/component/Offense.h>
#include <m2/box2d/Utils.h>
#include <m2g/SpriteBlueprint.h>

using namespace impl::object;

impl::object::Enemy::Enemy(m2::Object& obj, const character::CharacterBlueprint* blueprint) : character_state(blueprint), char_animator({obj.graphic(), blueprint}), fsm_variant(
	std::visit(overloaded {
		[&]([[maybe_unused]] const ai::type::ChaseBlueprint& v) -> FSMVariant { return m2::FSM<impl::fsm::Chaser>{{obj, blueprint->aiBlueprint}}; },
		[&]([[maybe_unused]] const ai::type::HitNRunBlueprint& v) -> FSMVariant { return m2::FSM<impl::fsm::Chaser>{{obj, blueprint->aiBlueprint}}; }, // TODO implement other FSMs
		[&]([[maybe_unused]] const ai::type::KeepDistanceBlueprint& v) -> FSMVariant { return m2::FSM<impl::fsm::Chaser>{{obj, blueprint->aiBlueprint}}; },
		[&]([[maybe_unused]] const ai::type::PatrolBlueprint& v) -> FSMVariant { return m2::FSM<impl::fsm::Chaser>{{obj, blueprint->aiBlueprint}}; }
	}, blueprint->aiBlueprint->variant)
), on_hit_color_mod_ttl(0) {}

M2Err Enemy::init(m2::Object& obj, const character::CharacterBlueprint* blueprint, m2::Vec2f pos) {
	obj = m2::Object{pos};

	auto& gfx = obj.add_graphic();
	gfx.textureRect = impl::sprites[blueprint->main_sprite_index].texture_rect;
	gfx.center_px = impl::sprites[blueprint->main_sprite_index].obj_center_px;

	auto& monitor = obj.add_monitor();

	auto& phy = obj.add_physique();
	phy.body = m2::box2d::create_dynamic_disk(
            *GAME.world,
            obj.physique_id,
            pos,
            false,
            true,
            m2::box2d::CATEGORY_ENEMY,
            std::get<m2::ColliderBlueprint::Circle>(impl::sprites[blueprint->main_sprite_index].collider.variant).radius_m,
			blueprint->mass_kg,
			blueprint->linear_damping
	);

	auto& def = obj.add_defense();
	def.hp = 100;
	def.maxHp = 100;

    obj.impl = std::make_unique<impl::object::Enemy>(obj, blueprint);

	monitor.pre_phy = [&]([[maybe_unused]] m2::component::Monitor& mon) {
		auto* data = dynamic_cast<Enemy*>(obj.impl.get());
		data->character_state.process_time(GAME.deltaTime_s);
		std::visit([](auto& v) { v.time(GAME.deltaTime_s); }, data->fsm_variant);
		std::visit([](auto& v) { v.signal(m2::FSMSIG_PREPHY); }, data->fsm_variant);
	};

	monitor.post_phy = [&]([[maybe_unused]] m2::component::Monitor& mon) {
		auto* data = dynamic_cast<Enemy*>(obj.impl.get());
		// We must call time before other signals
		data->char_animator.time(GAME.deltaTicks_ms / 1000.0f);
		m2::Vec2f velocity = m2::Vec2f{phy.body->GetLinearVelocity() };
		if (fabsf(velocity.x) < 0.5000f && fabsf(velocity.y) < 0.5000f) {
			data->char_animator.signal(impl::fsm::CharacterAnimation::CHARANIM_STOP);
		} else if (fabsf(velocity.x) < fabsf(velocity.y)) {
			if (0 < velocity.y) {
				data->char_animator.signal(impl::fsm::CharacterAnimation::CHARANIM_WALKDOWN);
			} else {
				data->char_animator.signal(impl::fsm::CharacterAnimation::CHARANIM_WALKUP);
			}
		} else {
			if (0 < velocity.x) {
				data->char_animator.signal(impl::fsm::CharacterAnimation::CHARANIM_WALKRIGHT);
			} else {
				data->char_animator.signal(impl::fsm::CharacterAnimation::CHARANIM_WALKLEFT);
			}
		}
	};

	gfx.on_draw = [&](m2::component::Graphic& gfx) {
		auto* data = dynamic_cast<Enemy*>(obj.impl.get());
		if (0.0f < data->on_hit_color_mod_ttl) {
			SDL_Texture *defaultTexture = gfx.texture;
			gfx.texture = GAME.sdlTextureMask;
			m2::component::Graphic::default_draw(gfx);
			gfx.texture = defaultTexture;
			data->on_hit_color_mod_ttl -= GAME.deltaTicks_ms / 1000.0f;
		} else {
			m2::component::Graphic::default_draw(gfx);
		}
		m2::component::Graphic::default_draw_healthbar(gfx, (float) def.hp / def.maxHp);
	};

	def.on_hit = [&]([[maybe_unused]] impl::component::Defense& def) {
		auto* data = dynamic_cast<Enemy*>(obj.impl.get());
		data->on_hit_color_mod_ttl = 0.10f;
	};

	def.on_death = [&](impl::component::Defense& def) {
		Game_DeleteList_Add(def.object_id);
	};

	return 0;
}
