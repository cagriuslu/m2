#include <m2/Object.h>
#include "m2/Def.h"
#include "m2/Game.hh"
#include <impl/private/object/Melee.h>
#include <impl/public/SpriteBlueprint.h>
#include <m2/box2d/Utils.h>

#define SWING_SPEED (15.0f)

static void Sword_prePhysics(m2::component::Monitor& mon) {
	auto& obj = GAME.objects[mon.object_id];
	auto& off = obj.offense();
	auto& melee_state = std::get<impl::character::MeleeState>(off.variant);

	melee_state.ttl_s -= GAME.deltaTicks_ms / 1000.0f;
	if (melee_state.ttl_s <= 0) {
		Game_DeleteList_Add(mon.object_id);
	}
}

static void Sword_postPhysics(m2::component::Monitor& mon) {
	auto& obj = GAME.objects[mon.object_id];
	auto& phy = obj.physique();
	float angle = phy.body->GetAngle();
	auto& gfx = obj.graphic();
	auto& off = obj.offense();

	m2::Object* originator = GAME.objects.get(off.originator);
	if (originator) {
		// Make sure originator is still alive
		phy.body->SetTransform(static_cast<b2Vec2>(originator->position), angle);
	}
	gfx.angle = angle;
}

static void Sword_onCollision(m2::component::Physique& phy, m2::component::Physique& other) {
	LOG_DEBUG("Collision");
	auto& obj = GAME.objects[phy.object_id];
	auto& other_obj = GAME.objects[other.object_id];
	auto& off = GAME.offenses[obj.offense_id];
	auto& melee_state = std::get<impl::character::MeleeState>(off.variant);
	auto& def = GAME.defenses[other_obj.defense_id];

	// Calculate damage
    def.hp -= melee_state.blueprint->damage;
	if (def.hp <= 0.0001f && def.onDeath) {
		LOG_TRACE_M2VV(M2_PROJECTILE_DEATH, ID, off.object_id, M2_ID, ID, def.object_id);
        def.onDeath(&def);
	} else {
		LOG_TRACE_M2VVV(M2_PROJECTILE_DMG, ID, off.object_id, M2_ID, ID, def.object_id, M2_HP, Float32, def.hp);
		auto direction = (other_obj.position - obj.position).normalize();
		auto force = direction * 15000.0f;
		other.body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
		if (def.onHit) {
            def.onHit(&def);
		}
	}
}

M2Err impl::object::Melee::init(m2::Object *obj, const character::MeleeBlueprint *blueprint, ID originatorId, m2::Vec2f position, m2::Vec2f direction) {
	*obj = m2::Object{position};

	const float theta = direction.angle_rads(); // Convert direction to angle
	const float startAngle = theta + SWING_SPEED * (150 / 1000.0f / 2.0f);

	auto& mon = obj->add_monitor();
	mon.prePhysics = Sword_prePhysics;
	mon.postPhysics = Sword_postPhysics;

	auto& phy = obj->add_physique();
	phy.body = m2::box2d::create_body(
            *GAME.world,
			obj->physique_id,
            false,
            true,
            position,
            false,
            true,
            true,
            originatorId == GAME.playerId ? m2::box2d::CATEGORY_PLAYER_MELEE_WEAPON : m2::box2d::CATEGORY_ENEMY_MELEE_WEAPON,
            0,
            m2::Vec2f{1.25f, 0.1667f},
            m2::Vec2f{0.5833f, 0.0f},
            0.0f,
            NAN,
            1.0f,
            0.0f,
            false
	);
	phy.body->SetTransform(static_cast<b2Vec2>(position), startAngle);
	phy.body->SetAngularVelocity(-SWING_SPEED);
	phy.onCollision = Sword_onCollision;

	auto& gfx = obj->add_graphic();
	gfx.textureRect = impl::sprites[blueprint->sprite_index].texture_rect;
	gfx.center_px = impl::sprites[blueprint->sprite_index].obj_center_px;
	gfx.angle = phy.body->GetAngle();

	auto& off = obj->add_offense();
    off.originator = originatorId;
	off.variant = blueprint->get_state();
	
	return 0;
}
