#include <m2/Object.h>
#include "m2/Game.hh"
#include <rpg/object/Melee.h>
#include <m2g/Object.h>
#include <m2/box2d/Utils.h>
#include <m2/Log.h>

#define SWING_SPEED (15.0f)

m2::VoidValue obj::Melee::init(m2::Object& obj, const chr::MeleeBlueprint *blueprint, m2::Id originatorId, m2::Vec2f direction) {
	const float theta = direction.angle_rads(); // Convert direction to angle
	const float startAngle = theta + SWING_SPEED * (150 / 1000.0f / 2.0f);

	auto& monitor = obj.add_monitor();

	auto& phy = obj.add_physique();

	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::DYNAMIC);
	bp.set_allow_sleep(false);
	bp.set_is_bullet(true);
	bp.mutable_foreground_fixture()->mutable_rect()->mutable_dims()->set_w(1.25f);
	bp.mutable_foreground_fixture()->mutable_rect()->mutable_dims()->set_h(0.1667f);
	bp.mutable_foreground_fixture()->mutable_rect()->mutable_center_offset()->set_x(0.5833f);
	bp.mutable_foreground_fixture()->mutable_rect()->mutable_center_offset()->set_y(0.0f);
	bp.mutable_foreground_fixture()->set_is_sensor(true);
	bp.mutable_foreground_fixture()->set_category(originatorId == GAME.playerId ? m2::pb::FixtureCategory::FRIEND_OFFENSE_ON_FOREGROUND : m2::pb::FixtureCategory::FOE_OFFENSE_ON_FOREGROUND);
	bp.set_mass(1.0f);
	bp.set_linear_damping(0);
	bp.set_fixed_rotation(false);
	phy.body = m2::box2d::create_body(*GAME.world, obj.physique_id(), obj.position, bp);
	phy.body->SetTransform(static_cast<b2Vec2>(obj.position), startAngle);
	phy.body->SetAngularVelocity(-SWING_SPEED);

	auto& gfx = obj.add_graphic(GAME.sprites[blueprint->sprite]);
	gfx.draw_angle = phy.body->GetAngle();

	auto& off = obj.add_offense();
    off.originator = originatorId;
	off.variant = chr::MeleeState(blueprint);

	monitor.pre_phy = [&](m2::comp::Monitor& mon) {
		auto& melee_state = std::get<chr::MeleeState>(off.variant);
		melee_state.ttl_s -= GAME.deltaTicks_ms / 1000.0f;
		if (melee_state.ttl_s <= 0) {
			GAME.add_deferred_action(m2::create_object_deleter(mon.object_id));
		}
	};

	monitor.post_phy = [&](MAYBE m2::comp::Monitor& mon) {
		float angle = phy.body->GetAngle();
		m2::Object* originator = GAME.objects.get(off.originator);
		if (originator) {
			// Make sure originator is still alive
			phy.body->SetTransform(static_cast<b2Vec2>(originator->position), angle);
		}
		gfx.draw_angle = angle;
	};

	phy.on_collision = [&](MAYBE m2::comp::Physique& phy, m2::comp::Physique& other) {
		LOG_DEBUG("Collision");
		auto& other_obj = GAME.objects[other.object_id];
		auto& melee_state = std::get<chr::MeleeState>(off.variant);
		auto& def = GAME.defenses[other_obj.defense_id()];

		// Calculate damage
		def.hp -= melee_state.blueprint->damage;
		if (def.hp <= 0.0001f && def.on_death) {
			LOG_TRACE("Projectile death", off.object_id, def.object_id);
			def.on_death(def);
		} else {
			LOG_TRACE("Projectile damage", off.object_id, def.object_id, def.hp);
			auto direction = (other_obj.position - obj.position).normalize();
			auto force = direction * 15000.0f;
			other.body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
			if (def.on_hit) {
				def.on_hit(def);
			}
		}
	};
	
	return {};
}
