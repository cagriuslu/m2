#include "../Object.h"
#include "../Main.h"
#include "../Item.h"
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

#define STOPWATCH_IDX_RANGED_ATTACK (0)
#define STOPWATCH_IDX_MELEE_ATTACK (1)
#define STOPWATCH_COUNT (2)

static void Player_prePhysics(ComponentEventListener* el) {
	Object* obj = Pool_GetById(&GAME->objects, el->super.objId);
	obj->properties->rangedAttackStopwatch += GAME->deltaTicks;
	obj->properties->meleeAttackStopwatch += GAME->deltaTicks;
	
	ComponentPhysics* phy = Pool_GetById(&GAME->physics, obj->physics);
	if (phy && phy->body) {
		Vec2F moveDirection = (Vec2F) {0.0f, 0.0f};
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

	if (GAME->events.buttonsPressed[BUTTON_SCROLL_DOWN]) {
		Item* curr, * next, * prev;
		if (GAME->events.keyStates[KEY_MODIFIER_SHIFT]) {
			LOG_INF("Switched to next secondary weapon");
			curr = Item_FindItemByTypeByFlags(&obj->properties->character->itemArray, ITEMTYP_SWORD | ITEMTYP_SPEAR | ITEMTYP_DAGGER, ITEMFLAG_EQUIPPED);
			next = Item_FindItemByTypeByFlags(&obj->properties->character->itemArray, ITEMTYP_SWORD | ITEMTYP_SPEAR | ITEMTYP_DAGGER, ITEMFLAG_PREEQUIPPED_NEXT);
			prev = Item_FindItemByTypeByFlags(&obj->properties->character->itemArray, ITEMTYP_SWORD | ITEMTYP_SPEAR | ITEMTYP_DAGGER, ITEMFLAG_PREEQUIPPED_PREV);
		} else {
			LOG_INF("Switched to next primary weapon");
			curr = Item_FindItemByTypeByFlags(&obj->properties->character->itemArray, ITEMTYP_GUN | ITEMTYP_RIFLE | ITEMTYP_BOW, ITEMFLAG_EQUIPPED);
			next = Item_FindItemByTypeByFlags(&obj->properties->character->itemArray, ITEMTYP_GUN | ITEMTYP_RIFLE | ITEMTYP_BOW, ITEMFLAG_PREEQUIPPED_NEXT);
			prev = Item_FindItemByTypeByFlags(&obj->properties->character->itemArray, ITEMTYP_GUN | ITEMTYP_RIFLE | ITEMTYP_BOW, ITEMFLAG_PREEQUIPPED_PREV);
		}
		curr->flags &= ~ITEMFLAG_EQUIPPED;
		curr->flags |= ITEMFLAG_PREEQUIPPED_PREV;
		next->flags &= ~ITEMFLAG_PREEQUIPPED_NEXT;
		next->flags |= ITEMFLAG_EQUIPPED;
		prev->flags &= ~ITEMFLAG_PREEQUIPPED_PREV;
		prev->flags |= ITEMFLAG_PREEQUIPPED_NEXT;
		Character_Preprocess(obj->properties->character);
	} else if (GAME->events.buttonsPressed[BUTTON_SCROLL_UP]) {
		Item* curr, * next, * prev;
		if (GAME->events.keyStates[KEY_MODIFIER_SHIFT]) {
			LOG_INF("Switched to previous secondary weapon");
			curr = Item_FindItemByTypeByFlags(&obj->properties->character->itemArray, ITEMTYP_SWORD | ITEMTYP_SPEAR | ITEMTYP_DAGGER, ITEMFLAG_EQUIPPED);
			next = Item_FindItemByTypeByFlags(&obj->properties->character->itemArray, ITEMTYP_SWORD | ITEMTYP_SPEAR | ITEMTYP_DAGGER, ITEMFLAG_PREEQUIPPED_NEXT);
			prev = Item_FindItemByTypeByFlags(&obj->properties->character->itemArray, ITEMTYP_SWORD | ITEMTYP_SPEAR | ITEMTYP_DAGGER, ITEMFLAG_PREEQUIPPED_PREV);
		} else {
			LOG_INF("Switched to previous primary weapon");
			curr = Item_FindItemByTypeByFlags(&obj->properties->character->itemArray, ITEMTYP_GUN | ITEMTYP_RIFLE | ITEMTYP_BOW, ITEMFLAG_EQUIPPED);
			next = Item_FindItemByTypeByFlags(&obj->properties->character->itemArray, ITEMTYP_GUN | ITEMTYP_RIFLE | ITEMTYP_BOW, ITEMFLAG_PREEQUIPPED_NEXT);
			prev = Item_FindItemByTypeByFlags(&obj->properties->character->itemArray, ITEMTYP_GUN | ITEMTYP_RIFLE | ITEMTYP_BOW, ITEMFLAG_PREEQUIPPED_PREV);
		}
		curr->flags &= ~ITEMFLAG_EQUIPPED;
		curr->flags |= ITEMFLAG_PREEQUIPPED_NEXT;
		next->flags &= ~ITEMFLAG_PREEQUIPPED_NEXT;
		next->flags |= ITEMFLAG_PREEQUIPPED_PREV;
		prev->flags &= ~ITEMFLAG_PREEQUIPPED_PREV;
		prev->flags |= ITEMFLAG_EQUIPPED;
		Character_Preprocess(obj->properties->character);
	} else {
		if (GAME->events.buttonStates[BUTTON_PRIMARY] && (100 < obj->properties->rangedAttackStopwatch)) {
			Vec2F pointerPosInWorld = CurrentPointerPositionInWorld();
			Vec2F bulletDir = Vec2F_Sub(pointerPosInWorld, obj->position);

			Object* bullet = Pool_Mark(&GAME->objects, NULL, NULL);
			Item* projectileWeapon = Item_FindItemByTypeByFlags(&obj->properties->character->itemArray, ITEMTYP_GUN | ITEMTYP_RIFLE | ITEMTYP_BOW, ITEMFLAG_EQUIPPED);
			ObjectBullet_Init(bullet, obj->position, bulletDir, projectileWeapon->type, &obj->properties->character->projectileOffense);
			obj->properties->rangedAttackStopwatch = 0;
		}

		if (GAME->events.buttonStates[BUTTON_SECONDARY] && (333 < obj->properties->meleeAttackStopwatch)) {
			Vec2F pointerPosInWorld = CurrentPointerPositionInWorld();
			Vec2F swordDir = Vec2F_Sub(pointerPosInWorld, obj->position);

			Object* sword = Pool_Mark(&GAME->objects, NULL, NULL);
			ObjectSword_Init(sword, obj->position, &obj->properties->character->meleeOffense, false, swordDir, 150);
			obj->properties->meleeAttackStopwatch = 0;
		}
	}
}

int ObjectPlayer_Init(Object* obj, Character* character) {
	REFLECT_ERROR(Object_Init(obj, (Vec2F) { 0.0f, 0.0f }, true));
	ID objId = Pool_GetId(&GAME->objects, obj);
	// Write-back originator ID of Character Offenses
	obj->properties->character = character;
	obj->properties->character->charOffense.originator = objId;
	Character_Preprocess(obj->properties->character);

	ComponentEventListener* el = Object_AddEventListener(obj, NULL);
	el->prePhysics = Player_prePhysics;

	TextureMap* textureMap = CurrentTextureMap();
	Texture* texture = HashMap_GetStringKey(&textureMap->lut, "playr00");

	ID phyId = 0;
	ComponentPhysics* phy = Object_AddPhysics(obj, &phyId);
	phy->body = Box2DUtils_CreateDynamicDisk(
		phyId,
		obj->position,
		false, // allowSleep
		CATEGORY_PLAYER,
		texture->diskRadius,
		4.0f, // Mass
		10.0f // Damping
	);

	ComponentGraphics* gfx = Object_AddGraphics(obj, NULL);
	gfx->txSrc = texture->rect;
	gfx->txCenter = texture->center;

	ComponentDefense* def = Object_AddDefense(obj, NULL);
	ComponentDefense_CopyExceptSuper(def, &obj->properties->character->defense);
	def->hp = def->maxHp;

	return 0;
}
