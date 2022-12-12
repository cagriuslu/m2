#include <m2/Object.h>
#include "m2/Game.hh"
#include <rpg/object/Projectile.h>
#include <m2g/Object.h>
#include <m2/box2d/Utils.h>
#include <m2/M2.h>
#include <m2/Log.h>

m2::VoidValue obj::Projectile::init(m2::Object& obj, const chr::ProjectileBlueprint* blueprint, m2::Vec2f dir) {
	dir = dir.normalize();

	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::DYNAMIC);
	bp.set_allow_sleep(false);
	bp.set_is_bullet(true);
	bp.mutable_foreground_fixture()->mutable_circ()->set_radius(0.167f);
	bp.mutable_foreground_fixture()->set_is_sensor(true);
	bp.mutable_foreground_fixture()->set_category(m2::pb::FixtureCategory::FRIEND_OFFENSE_ON_FOREGROUND);
	bp.set_mass(0);
	bp.set_linear_damping(0);
	bp.set_fixed_rotation(true);
	phy.body = m2::box2d::create_body(*GAME.world, obj.physique_id(), obj.position, bp);
	phy.body->SetLinearVelocity(static_cast<b2Vec2>(dir * blueprint->speed_mps));
	phy.pre_step = [&obj](m2::Physique& phy) {
		auto& off = obj.offense();
		auto& projectile_state = std::get<chr::ProjectileState>(off.variant);
		m2::Vec2f curr_direction = m2::Vec2f{phy.body->GetLinearVelocity() }.normalize();
		phy.body->SetLinearVelocity(static_cast<b2Vec2>(curr_direction * projectile_state.blueprint->speed_mps));

		projectile_state.ttl_s -= GAME.deltaTicks_ms / 1000.0f;
		if (projectile_state.ttl_s <= 0) {
			GAME.add_deferred_action(m2::create_object_deleter(phy.object_id));
		}
	};

	auto& gfx = obj.add_graphic(GAME.sprites[blueprint->sprite]);
	gfx.draw_angle = dir.angle_rads();

	auto& chr = obj.add_tiny_character();

	auto& off = obj.add_offense();
	off.variant = chr::ProjectileState(blueprint);

	phy.on_collision = [&off](m2::Physique& phy, m2::Physique& other) {
		auto& other_obj = GAME.objects[other.object_id];
		auto& projectile_state = std::get<chr::ProjectileState>(off.variant);
		auto* def = GAME.defenses.get(other_obj.defense_id());
		if (def) {
			// Check if already collided
			if (projectile_state.already_collided_this_step) {
				return;
			}
			projectile_state.already_collided_this_step = true;
			// Calculate damage
			def->hp -= m2::apply_accuracy(projectile_state.blueprint->damage, projectile_state.blueprint->damage_accuracy);
			if (def->hp <= 0.0001f && def->on_death) {
				LOG_TRACE("Projectile death", off.object_id, def->object_id);
				def->on_death(*def);
			} else {
				LOG_TRACE("Projectile damage", off.object_id, def->object_id, def->hp);
				auto direction = m2::Vec2f{phy.body->GetLinearVelocity() }.normalize();
				auto force = direction * 5000.0f;
				other.body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
				if (def->on_hit) {
					def->on_hit(*def);
				}
			}
			GAME.add_deferred_action(m2::create_object_deleter(phy.object_id));
		}
	};

	return {};
}
