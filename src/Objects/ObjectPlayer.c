#include "../Object.h"
#include "../Game.h"
#include "../Box2DUtils.h"
#include "../Controls.h"
#include "../Event.h"
#include "../Log.h"
#include <stdio.h>

// Mouse primary button: shoot projectile (player can at most carry 3 primary weapons)
// Mouse secondary button: melee weapon (player can only carry one melee weapon)
// Mouse middle button: explosive weapon (player can only carry one explosive weapon)
// Mouse middle scroll: change primary projectile weapon
// Double tap directional buttons to dodge

static void Player_prePhysics(ComponentEventListener* el) {
	Object* obj = Pool_GetById(&GAME->objects, el->super.objId);

	if (obj->ex && obj->ex->type == CFG_OBJTYP_PLAYER) {
		obj->ex->value.player.rangedAttackStopwatch += GAME->deltaTicks;
		obj->ex->value.player.meleeAttackStopwatch += GAME->deltaTicks;

		ComponentPhysics* phy = Pool_GetById(&GAME->physics, obj->physics);
		if (phy && phy->body) {
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
		}

		if (GAME->events.buttonStates[BUTTON_PRIMARY] && (100 < obj->ex->value.player.rangedAttackStopwatch)) {
			Object* projectile = Pool_Mark(&GAME->objects, NULL, NULL);
			ObjectProjectile_InitFromCfg(projectile, &obj->ex->value.player.chr->defaultRangedWeapon->projectile, GAME->playerId, obj->position, Vec2F_Sub(CurrentPointerPositionInWorld(), obj->position));
			obj->ex->value.player.rangedAttackStopwatch = 0;
		}
		if (GAME->events.buttonStates[BUTTON_SECONDARY] && (333 < obj->ex->value.player.meleeAttackStopwatch)) {
			Object* melee = Pool_Mark(&GAME->objects, NULL, NULL);
			ObjectMelee_InitFromCfg(melee, &obj->ex->value.player.chr->defaultMeleeWeapon->melee, GAME->playerId, obj->position, Vec2F_Sub(CurrentPointerPositionInWorld(), obj->position));
			obj->ex->value.player.meleeAttackStopwatch = 0;
		}
	}
}

int ObjectPlayer_InitFromCfg(Object* obj, const CfgCharacter *cfg, Vec2F position) {
	REFLECT_ERROR(Object_Init(obj, position, true));
	obj->ex->type = CFG_OBJTYP_PLAYER;
	obj->ex->value.player.chr = cfg;
	// TODO implement CharacterState

	ComponentEventListener* el = Object_AddEventListener(obj);
	el->prePhysics = Player_prePhysics;

	ComponentPhysics* phy = Object_AddPhysics(obj);
	phy->body = Box2DUtils_CreateDynamicDisk(
		Pool_GetId(&GAME->physics, phy),
		obj->position,
		false, // allowSleep
		CATEGORY_PLAYER,
		cfg->texture->collider.colliderUnion.circ.radius_m,
		4.0f, // Mass
		10.0f // Damping
	);

	ComponentGraphics* gfx = Object_AddGraphics(obj);
	gfx->textureRect = cfg->texture->textureRect;
	gfx->center_px = cfg->texture->objCenter_px;

	ComponentDefense* def = Object_AddDefense(obj);
	def->maxHp = def->hp = cfg->maxHp;

	GAME->playerId = Pool_GetId(&GAME->objects, obj);
	return XOK;
}
