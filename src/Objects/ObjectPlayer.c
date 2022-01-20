#include "../Object.h"
#include "../Game.h"
#include "../Box2DUtils.h"
#include "../Controls.h"
#include "../Event.h"
#include "../Def.h"

// Mouse primary button: shoot projectile (player can at most carry 3 primary weapons)
// Mouse secondary button: melee weapon (player can only carry one melee weapon)
// Mouse middle button: explosive weapon (player can only carry one explosive weapon)
// Mouse middle scroll: change primary projectile weapon
// Double tap directional buttons to dodge

static void Player_prePhysics(ComponentMonitor* el) {
	Object* obj = Pool_GetById(&GAME->objects, el->super.objId); XASSERT(obj);

	obj->ex->value.player.rangedAttackStopwatch += GAME->deltaTicks;
	obj->ex->value.player.meleeAttackStopwatch += GAME->deltaTicks;

	ComponentPhysique* phy = Pool_GetById(&GAME->physics, obj->physique); XASSERT(phy);
	Vec2F moveDirection = (Vec2F){ 0.0f, 0.0f };
	if (GAME->events.keyStates[KEY_UP]) {
		moveDirection.y += -1.0f;
	}
	if (GAME->events.keyStates[KEY_DOWN]) {
		moveDirection.y += 1.0f;
	}
	if (GAME->events.keyStates[KEY_LEFT]) {
		moveDirection.x += -1.0f;
	}
	if (GAME->events.keyStates[KEY_RIGHT]) {
		moveDirection.x += 1.0f;
	}
	Box2DBodyApplyForceToCenter(phy->body, Vec2F_Mul(Vec2F_Normalize(moveDirection), GAME->deltaTicks * 25.0f), true);

	if (GAME->events.buttonStates[BUTTON_PRIMARY] && (100 < obj->ex->value.player.rangedAttackStopwatch)) {
		Object* projectile = Pool_Mark(&GAME->objects, NULL, NULL);
		ObjectProjectile_InitFromCfg(projectile, &obj->ex->value.player.chr->defaultRangedWeapon->projectile, GAME->playerId, obj->position, Vec2F_Sub(GAME->mousePositionInWorld, obj->position));
		obj->ex->value.player.rangedAttackStopwatch = 0;
	}
	if (GAME->events.buttonStates[BUTTON_SECONDARY] && (333 < obj->ex->value.player.meleeAttackStopwatch)) {
		Object* melee = Pool_Mark(&GAME->objects, NULL, NULL);
		ObjectMelee_InitFromCfg(melee, &obj->ex->value.player.chr->defaultMeleeWeapon->melee, GAME->playerId, obj->position, Vec2F_Sub(GAME->mousePositionInWorld, obj->position));
		obj->ex->value.player.meleeAttackStopwatch = 0;
	}
	if (GAME->events.buttonsPressed[BUTTON_MIDDLE]) {
		Object* explosive = Pool_Mark(&GAME->objects, NULL, NULL);
		ObjectExplosive_InitFromCfg(explosive, &obj->ex->value.player.chr->defaultExplosiveWeapon->explosive, GAME->playerId, obj->position, Vec2F_Sub(GAME->mousePositionInWorld, obj->position));
	}
}

static void Player_onDeath(ComponentDefense *def) {
	LOG_INF("Player died");
}

static void Player_postPhysics(ComponentMonitor* monitor) {
	Object* obj = Pool_GetById(&GAME->objects, monitor->super.objId); XASSERT(obj);
	ComponentPhysique* phy = Pool_GetById(&GAME->physics, obj->physique); XASSERT(phy);
	// We must call time before other signals
	StateMachine_ProcessTime(&obj->ex->value.player.stateMachineCharacterAnimation, GAME->deltaTicks / 1000.0f);
	Vec2F velocity = Box2DBodyGetLinearVelocity(phy->body);
	if (fabsf(velocity.x) < 0.5000f && fabsf(velocity.y) < 0.5000f) {
		StateMachine_ProcessSignal(&obj->ex->value.player.stateMachineCharacterAnimation, SIG_CHARANIM_STOP);
	} else if (fabsf(velocity.x) < fabsf(velocity.y)) {
		if (0 < velocity.y) {
			StateMachine_ProcessSignal(&obj->ex->value.player.stateMachineCharacterAnimation, SIG_CHARANIM_WALKDOWN);
		} else {
			StateMachine_ProcessSignal(&obj->ex->value.player.stateMachineCharacterAnimation, SIG_CHARANIM_WALKUP);
		}
	} else {
		if (0 < velocity.x) {
			StateMachine_ProcessSignal(&obj->ex->value.player.stateMachineCharacterAnimation, SIG_CHARANIM_WALKRIGHT);
		} else {
			StateMachine_ProcessSignal(&obj->ex->value.player.stateMachineCharacterAnimation, SIG_CHARANIM_WALKLEFT);
		}
	}
}

int ObjectPlayer_InitFromCfg(Object* obj, const CfgCharacter *cfg, Vec2F position) {
	XERR_REFLECT(Object_Init(obj, position, true));
	obj->ex->type = CFG_OBJTYP_PLAYER;
	obj->ex->value.player.chr = cfg;
	// TODO implement CharacterState

	ComponentMonitor* el = Object_AddMonitor(obj);
	el->prePhysics = Player_prePhysics;
	el->postPhysics = Player_postPhysics;

	ComponentPhysique* phy = Object_AddPhysique(obj);
	phy->body = Box2DUtils_CreateDynamicDisk(
		Pool_GetId(&GAME->physics, phy),
		obj->position,
		false, // allowSleep
		CATEGORY_PLAYER,
		cfg->mainTexture->collider.colliderUnion.circ.radius_m,
		4.0f, // Mass
		10.0f // Damping
	);

	ComponentGraphic* gfx = Object_AddGraphic(obj);
	gfx->textureRect = cfg->mainTexture->textureRect;
	gfx->center_px = cfg->mainTexture->objCenter_px;

	ComponentDefense* def = Object_AddDefense(obj);
	def->maxHp = def->hp = cfg->maxHp;
	def->onDeath = Player_onDeath;

	StateMachineCharacterAnimation_Init(&obj->ex->value.player.stateMachineCharacterAnimation, cfg, gfx);

	GAME->playerId = Pool_GetId(&GAME->objects, obj);
	return XOK;
}
