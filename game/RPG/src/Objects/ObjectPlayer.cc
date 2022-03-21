#include <m2/object/Object.hh>
#include "m2/Game.hh"
#include "m2/Box2DUtils.hh"
#include "m2/Controls.hh"
#include "m2/Event.hh"
#include "m2/Def.hh"
#include "../ARPG_Object.hh"
#include <game/ARPG_Cfg.hh>
#include <game/component.hh>

// Mouse primary button: shoot projectile (player can at most carry 3 primary weapons)
// Mouse secondary button: melee weapon (player can only carry one melee weapon)
// Mouse middle button: explosive weapon (player can only carry one explosive weapon)
// Mouse middle scroll: change primary projectile weapon
// Double tap directional buttons to dodge

static void Player_prePhysics(ComponentMonitor* el) {
	Object* obj = GAME.objects.get(el->super.objId);
	PlayerData *playerData = AS_PLAYERDATA(obj->data);

	m2::vec2f moveDirection;
	if (GAME.events.keyStates[KEY_UP]) {
		moveDirection.y += -1.0f;
		Automaton_ProcessSignal(&playerData->charAnimationAutomaton, SIG_CHARANIM_WALKUP);
	}
	if (GAME.events.keyStates[KEY_DOWN]) {
		moveDirection.y += 1.0f;
		Automaton_ProcessSignal(&playerData->charAnimationAutomaton, SIG_CHARANIM_WALKDOWN);
	}
	if (GAME.events.keyStates[KEY_LEFT]) {
		moveDirection.x += -1.0f;
		Automaton_ProcessSignal(&playerData->charAnimationAutomaton, SIG_CHARANIM_WALKLEFT);
	}
	if (GAME.events.keyStates[KEY_RIGHT]) {
		moveDirection.x += 1.0f;
		Automaton_ProcessSignal(&playerData->charAnimationAutomaton, SIG_CHARANIM_WALKRIGHT);
	}

	ComponentPhysique* phy = GAME.physics.get(obj->physique);
	phy->body->ApplyForceToCenter(static_cast<b2Vec2>(moveDirection.normalize() * (GAME.deltaTicks_ms * 25.0f)), true);

	playerData->characterState.rangedWeaponState.cooldownCounter_s += GAME.deltaTime_s;
	playerData->characterState.meleeWeaponState.cooldownCounter_s += GAME.deltaTime_s;
	playerData->characterState.explosiveWeaponState.cooldownCounter_s += GAME.deltaTime_s;

	if (GAME.events.buttonStates[BUTTON_PRIMARY] && playerData->characterState.rangedWeaponState.cfg->cooldown_s < playerData->characterState.rangedWeaponState.cooldownCounter_s) {
		Object* projectile = &GAME.objects.alloc().first;
		m2::vec2f direction = (GAME.mousePositionInWorld_m - obj->position).normalize();
		float accuracy = playerData->characterState.cfg->defaultRangedWeapon->accuracy;
		float angle = direction.angle_rads() + (M2_PI * randf() * (1 - accuracy)) - (M2_PI * ((1 - accuracy) / 2.0f));
		ObjectProjectile_InitFromCfg(projectile, &playerData->characterState.cfg->defaultRangedWeapon->projectile, GAME.playerId, obj->position, m2::vec2f::from_angle(angle));
		// Knockback
		phy->body->ApplyForceToCenter(static_cast<b2Vec2>(m2::vec2f::from_angle(angle + M2_PI) * 500.0f), true);
		// TODO set looking direction here as well
		playerData->characterState.rangedWeaponState.cooldownCounter_s = 0;
	}
	if (GAME.events.buttonStates[BUTTON_SECONDARY] && playerData->characterState.meleeWeaponState.cfg->cooldown_s < playerData->characterState.meleeWeaponState.cooldownCounter_s) {
		Object* melee = &GAME.objects.alloc().first;
		ObjectMelee_InitFromCfg(melee, &playerData->characterState.cfg->defaultMeleeWeapon->melee, GAME.playerId, obj->position, GAME.mousePositionInWorld_m - obj->position);
		playerData->characterState.meleeWeaponState.cooldownCounter_s = 0;
	}
	if (GAME.events.buttonStates[BUTTON_MIDDLE] && playerData->characterState.explosiveWeaponState.cfg->cooldown_s < playerData->characterState.explosiveWeaponState.cooldownCounter_s) {
		Object* explosive = &GAME.objects.alloc().first;
		ObjectExplosive_InitFromCfg(explosive, &playerData->characterState.cfg->defaultExplosiveWeapon->explosive, GAME.playerId, obj->position, GAME.mousePositionInWorld_m - obj->position);
		playerData->characterState.explosiveWeaponState.cooldownCounter_s = 0;
	}
}

static void Player_onDeath(game::component_defense *def) {
	LOG_INFO("Player died");
}

static void Player_postPhysics(ComponentMonitor* monitor) {
	Object* obj = GAME.objects.get(monitor->super.objId);
	PlayerData *playerData = AS_PLAYERDATA(obj->data);
	ComponentPhysique* phy = GAME.physics.get(obj->physique);

	// We must call time before other signals
	Automaton_ProcessTime(&playerData->charAnimationAutomaton, GAME.deltaTime_s);
	m2::vec2f velocity = m2::vec2f{ phy->body->GetLinearVelocity() };
	if (fabsf(velocity.x) < 0.5000f && fabsf(velocity.y) < 0.5000f) {
		Automaton_ProcessSignal(&playerData->charAnimationAutomaton, SIG_CHARANIM_STOP);
	}
}

int ObjectPlayer_InitFromCfg(Object* obj, const CfgCharacter *cfg, m2::vec2f position) {
	M2ERR_REFLECT(Object_Init(obj, position));
	obj->data = calloc(1, sizeof(PlayerData)); M2ASSERT(obj->data);
	PlayerData *playerData = AS_PLAYERDATA(obj->data);
	M2ERR_REFLECT(CharacterState_Init(&playerData->characterState, cfg));

	ComponentMonitor* el = Object_AddMonitor(obj);
	el->prePhysics = Player_prePhysics;
	el->postPhysics = Player_postPhysics;

	ComponentPhysique* phy = Object_AddPhysique(obj);
	ID phyId = GAME.physics.get_id(phy);
	phy->body = Box2DUtils_CreateDynamicDisk(
		phyId,
		obj->position,
		false, // allowSleep
		CATEGORY_PLAYER,
		ARPG_CFG_SPRITES[cfg->mainSpriteIndex].collider.params.circ.radius_m,
		cfg->mass_kg,
		cfg->linearDamping
	);

	ComponentGraphic* gfx = Object_AddGraphic(obj);
	gfx->textureRect = ARPG_CFG_SPRITES[cfg->mainSpriteIndex].textureRect;
	gfx->center_px = ARPG_CFG_SPRITES[cfg->mainSpriteIndex].objCenter_px;

	ComponentLight* light = Object_AddLight(obj);
	light->radius_m = 4.0f;

	game::component_defense* def = Object_AddDefense(obj);
    def->maxHp = def->hp = cfg->maxHp;
    def->onDeath = Player_onDeath;

	AutomatonCharAnimation_Init(&playerData->charAnimationAutomaton, cfg, gfx);

	GAME.playerId = GAME.objects.get_id(obj);
	return M2OK;
}
