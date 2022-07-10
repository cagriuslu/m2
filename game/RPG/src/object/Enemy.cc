#include <rpg/object/Enemy.h>
#include <rpg/object/ConsumableDrop.h>
#include <m2/Object.h>
#include "m2/Game.hh"
#include "m2/Def.h"
#include <m2/M2.h>
#include <rpg/ConsumableBlueprint.h>
#include <m2g/component/Defense.h>
#include <m2/box2d/Utils.h>
#include <m2g/SpriteBlueprint.h>
#include <rpg/LevelBlueprint.h>
#include <m2/Group.h>
#include <deque>

struct ConsumableResourceGroup : public m2::Group {
	std::deque<const itm::ConsumableBlueprint*> consumables;

	explicit ConsumableResourceGroup(decltype(consumables)&& consumables) : m2::Group(), consumables(consumables) {}

	const itm::ConsumableBlueprint* pop_consumable() {
		if (not consumables.empty() && m2::rand(members().size()) == 0) {
			auto pop_index = m2::rand(static_cast<uint32_t>(consumables.size()));
			const auto* consumable = consumables[pop_index];
			consumables.erase(consumables.begin() + pop_index);
			return consumable;
		}
		return nullptr;
	}
};

using namespace obj;

obj::Enemy::Enemy(m2::Object& obj, const chr::CharacterBlueprint* blueprint) : character_state(blueprint),
	char_animator({obj.graphic(), blueprint}), fsm_variant(
		std::visit(overloaded {
			[&](MAYBE const ai::type::ChaseBlueprint& v) -> FSMVariant { return m2::FSM<fsm::Chaser>{{obj, blueprint->aiBlueprint}}; },
			[&](MAYBE const ai::type::HitNRunBlueprint& v) -> FSMVariant { return m2::FSM<fsm::Chaser>{{obj, blueprint->aiBlueprint}}; }, // TODO implement other FSMs
			[&](MAYBE const ai::type::KeepDistanceBlueprint& v) -> FSMVariant { return m2::FSM<fsm::Chaser>{{obj, blueprint->aiBlueprint}}; },
			[&](MAYBE const ai::type::PatrolBlueprint& v) -> FSMVariant { return m2::FSM<fsm::Chaser>{{obj, blueprint->aiBlueprint}}; }
		}, blueprint->aiBlueprint->variant)), on_hit_color_mod_ttl(0) {}

void Enemy::stun() {
	character_state.stun();
}

m2::VoidValue Enemy::init(m2::Object& obj, const chr::CharacterBlueprint* blueprint, m2::GroupID group_id, m2::Vec2f pos) {
	obj = m2::Object{pos};

	if (group_id.type) {
		obj.add_to_group(group_id, [=]() -> std::unique_ptr<m2::Group> {
			switch (group_id.type) {
				case lvl::CONSUMABLE_RESOURCE_GROUP_HP:
					return std::make_unique<ConsumableResourceGroup>(ConsumableResourceGroup{{&itm::health_drop_20}});
				default:
					return {};
			}
		});
	}

	auto& gfx = obj.add_graphic();
	gfx.textureRect = m2g::sprites[blueprint->main_sprite_index].texture_rect;
	gfx.center_px = m2g::sprites[blueprint->main_sprite_index].obj_center_px;

	auto& monitor = obj.add_monitor();

	auto& phy = obj.add_physique();
	phy.body = m2::box2d::create_dynamic_disk(
            *GAME.world,
            obj.physique_id(),
            pos,
            false,
            true,
            m2::box2d::CAT_ENEMY,
            std::get<m2::ColliderBlueprint::Circle>(m2g::sprites[blueprint->main_sprite_index].collider.variant).radius_m,
			blueprint->mass_kg,
			blueprint->linear_damping
	);

	auto& def = obj.add_defense();
	def.hp = 100;
	def.maxHp = 100;

    obj.impl = std::make_unique<obj::Enemy>(obj, blueprint);

	monitor.pre_phy = [&](MAYBE m2::comp::Monitor& mon) {
		auto* impl = dynamic_cast<Enemy*>(obj.impl.get());
		impl->character_state.process_time(GAME.deltaTime_s);
		std::visit([](auto& v) { v.time(GAME.deltaTime_s); }, impl->fsm_variant);
		std::visit([](auto& v) { v.signal(m2::FSMSIG_PREPHY); }, impl->fsm_variant);
	};

	monitor.post_phy = [&](MAYBE m2::comp::Monitor& mon) {
		auto* data = dynamic_cast<Enemy*>(obj.impl.get());
		// We must call time before other signals
		data->char_animator.time(GAME.deltaTicks_ms / 1000.0f);
		m2::Vec2f velocity = m2::Vec2f{phy.body->GetLinearVelocity() };
		if (fabsf(velocity.x) < 0.5000f && fabsf(velocity.y) < 0.5000f) {
			data->char_animator.signal(fsm::CharacterAnimation::CHARANIM_STOP);
		} else if (fabsf(velocity.x) < fabsf(velocity.y)) {
			if (0 < velocity.y) {
				data->char_animator.signal(fsm::CharacterAnimation::CHARANIM_WALKDOWN);
			} else {
				data->char_animator.signal(fsm::CharacterAnimation::CHARANIM_WALKUP);
			}
		} else {
			if (0 < velocity.x) {
				data->char_animator.signal(fsm::CharacterAnimation::CHARANIM_WALKRIGHT);
			} else {
				data->char_animator.signal(fsm::CharacterAnimation::CHARANIM_WALKLEFT);
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

		auto gid = obj.group_id();
		if (lvl::CONSUMABLE_RESOURCE_GROUP < gid.type && gid.type < lvl::CONSUMABLE_RESOURCE_GROUP_N) {
			auto& resource_group = dynamic_cast<ConsumableResourceGroup&>(obj.group());
			const auto* consumable = resource_group.pop_consumable();
			if (consumable) {
				GAME.add_deferred_action([=]() {
					ConsumableDrop::init(GAME.objects.alloc().first, *consumable, drop_position);
				});
			}
		}
		// Delete self
		GAME.add_deferred_action(m2::create_object_deleter(def.object_id));
	};

	return {};
}
