#include "../Object.h"
#include "../Main.h"
#include "../Item.h"
#include "../Box2DUtils.h"
#include "../Controls.h"
#include "../Event.h"
#include "../Log.h"
#include <stdio.h>

// Shoot with mouse primary and secondary
// Change primary skill with wheel
// Change secondary skill with middle button
// Shift button is also used during combat
// Ctrl and Alt should not be used during combat

#define STOPWATCH_IDX_RANGED_ATTACK (0)
#define STOPWATCH_IDX_MELEE_ATTACK (1)
#define STOPWATCH_COUNT (2)

static void Player_prePhysics(ComponentEventListener* el) {
	Object* obj = Bucket_GetById(&CurrentLevel()->objects, el->super.objId);
	ComponentPhysics* phy = Bucket_GetById(&CurrentLevel()->physics, obj->physics);
	if (phy && phy->body) {
		Vec2F moveDirection = (Vec2F) {0.0f, 0.0f};
		if (IsKeyDown(KEY_UP)) {
			moveDirection.y += -1.0f;
		}
		if (IsKeyDown(KEY_DOWN)) {
			moveDirection.y += 1.0f;
		}
		if (IsKeyDown(KEY_LEFT)) {
			moveDirection.x += -1.0f;
		}
		if (IsKeyDown(KEY_RIGHT)) {
			moveDirection.x += 1.0f;
		}
		Box2DBodyApplyForceToCenter(phy->body, Vec2F_Mul(Vec2F_Normalize(moveDirection), DeltaTicks() * 25.0f), true);
	}

	if (IsButtonPressed(BUTTON_SCROLL_DOWN)) {
		Item* curr, * next, * prev;
		if (IsKeyDown(KEY_MODIFIER_SHIFT)) {
			LOG_INF("Switched to next secondary weapon");
			curr = Item_FindItemByTypeByFlags(&CurrentCharacter()->itemArray, ITEMTYP_SWORD | ITEMTYP_SPEAR | ITEMTYP_DAGGER, ITEMFLAG_EQUIPPED);
			next = Item_FindItemByTypeByFlags(&CurrentCharacter()->itemArray, ITEMTYP_SWORD | ITEMTYP_SPEAR | ITEMTYP_DAGGER, ITEMFLAG_PREEQUIPPED_NEXT);
			prev = Item_FindItemByTypeByFlags(&CurrentCharacter()->itemArray, ITEMTYP_SWORD | ITEMTYP_SPEAR | ITEMTYP_DAGGER, ITEMFLAG_PREEQUIPPED_PREV);
		} else {
			LOG_INF("Switched to next primary weapon");
			curr = Item_FindItemByTypeByFlags(&CurrentCharacter()->itemArray, ITEMTYP_GUN | ITEMTYP_RIFLE | ITEMTYP_BOW, ITEMFLAG_EQUIPPED);
			next = Item_FindItemByTypeByFlags(&CurrentCharacter()->itemArray, ITEMTYP_GUN | ITEMTYP_RIFLE | ITEMTYP_BOW, ITEMFLAG_PREEQUIPPED_NEXT);
			prev = Item_FindItemByTypeByFlags(&CurrentCharacter()->itemArray, ITEMTYP_GUN | ITEMTYP_RIFLE | ITEMTYP_BOW, ITEMFLAG_PREEQUIPPED_PREV);
		}
		curr->flags &= ~ITEMFLAG_EQUIPPED;
		curr->flags |= ITEMFLAG_PREEQUIPPED_PREV;
		next->flags &= ~ITEMFLAG_PREEQUIPPED_NEXT;
		next->flags |= ITEMFLAG_EQUIPPED;
		prev->flags &= ~ITEMFLAG_PREEQUIPPED_PREV;
		prev->flags |= ITEMFLAG_PREEQUIPPED_NEXT;
		Character_Preprocess(CurrentCharacter());
	} else if (IsButtonPressed(BUTTON_SCROLL_UP)) {
		Item* curr, * next, * prev;
		if (IsKeyDown(KEY_MODIFIER_SHIFT)) {
			LOG_INF("Switched to previous secondary weapon");
			curr = Item_FindItemByTypeByFlags(&CurrentCharacter()->itemArray, ITEMTYP_SWORD | ITEMTYP_SPEAR | ITEMTYP_DAGGER, ITEMFLAG_EQUIPPED);
			next = Item_FindItemByTypeByFlags(&CurrentCharacter()->itemArray, ITEMTYP_SWORD | ITEMTYP_SPEAR | ITEMTYP_DAGGER, ITEMFLAG_PREEQUIPPED_NEXT);
			prev = Item_FindItemByTypeByFlags(&CurrentCharacter()->itemArray, ITEMTYP_SWORD | ITEMTYP_SPEAR | ITEMTYP_DAGGER, ITEMFLAG_PREEQUIPPED_PREV);
		} else {
			LOG_INF("Switched to previous primary weapon");
			curr = Item_FindItemByTypeByFlags(&CurrentCharacter()->itemArray, ITEMTYP_GUN | ITEMTYP_RIFLE | ITEMTYP_BOW, ITEMFLAG_EQUIPPED);
			next = Item_FindItemByTypeByFlags(&CurrentCharacter()->itemArray, ITEMTYP_GUN | ITEMTYP_RIFLE | ITEMTYP_BOW, ITEMFLAG_PREEQUIPPED_NEXT);
			prev = Item_FindItemByTypeByFlags(&CurrentCharacter()->itemArray, ITEMTYP_GUN | ITEMTYP_RIFLE | ITEMTYP_BOW, ITEMFLAG_PREEQUIPPED_PREV);
		}
		curr->flags &= ~ITEMFLAG_EQUIPPED;
		curr->flags |= ITEMFLAG_PREEQUIPPED_NEXT;
		next->flags &= ~ITEMFLAG_PREEQUIPPED_NEXT;
		next->flags |= ITEMFLAG_PREEQUIPPED_PREV;
		prev->flags &= ~ITEMFLAG_PREEQUIPPED_PREV;
		prev->flags |= ITEMFLAG_EQUIPPED;
		Character_Preprocess(CurrentCharacter());
	} else {
		Stopwatch* rangedAttackStopwatch = Object_GetPrePhysicsStopwatchPtr(obj, STOPWATCH_IDX_RANGED_ATTACK);
		if (IsButtonDown(BUTTON_PRIMARY) && (100 < *rangedAttackStopwatch)) {
			Vec2F pointerPosInWorld = CurrentPointerPositionInWorld();
			Vec2F bulletDir = Vec2F_Sub(pointerPosInWorld, obj->position);

			Object* bullet = Bucket_Mark(&CurrentLevel()->objects, NULL, NULL);
			Item* projectileWeapon = Item_FindItemByTypeByFlags(&CurrentCharacter()->itemArray, ITEMTYP_GUN | ITEMTYP_RIFLE | ITEMTYP_BOW, ITEMFLAG_EQUIPPED);
			ObjectBullet_Init(bullet, obj->position, bulletDir, projectileWeapon->type, &CurrentCharacter()->projectileOffense);
			*rangedAttackStopwatch = 0;
		}

		Stopwatch* meleeAttackStopwatch = Object_GetPrePhysicsStopwatchPtr(obj, STOPWATCH_IDX_MELEE_ATTACK);
		if (IsButtonDown(BUTTON_SECONDARY) && (333 < *meleeAttackStopwatch)) {
			Vec2F pointerPosInWorld = CurrentPointerPositionInWorld();
			Vec2F swordDir = Vec2F_Sub(pointerPosInWorld, obj->position);

			Object* sword = Bucket_Mark(&CurrentLevel()->objects, NULL, NULL);
			ObjectSword_Init(sword, obj->position, &CurrentCharacter()->meleeOffense, swordDir, 150);
			*meleeAttackStopwatch = 0;
		}
	}
}

int ObjectPlayer_Init(Object* obj) {
	PROPAGATE_ERROR(Object_Init(obj, (Vec2F) { 0.0f, 0.0f }));
	ID objId = Bucket_GetId(&CurrentLevel()->objects, obj);
	// Write-back originator ID of Character Offenses
	CurrentCharacter()->charOffense.originator = objId;
	Character_Preprocess(CurrentCharacter());

	ComponentEventListener* el = Object_AddEventListener(obj, NULL);
	el->prePhysics = Player_prePhysics;

	ID phyId = 0;
	ComponentPhysics* phy = Object_AddPhysics(obj, &phyId);
	phy->body = Box2DUtils_CreateDynamicDisk(
		phyId,
		obj->position,
		false, // allowSleep
		CATEGORY_PLAYER,
		0.229167f, // Radius
		4.0f, // Mass
		10.0f // Damping
	);

	ComponentGraphics* gfx = Object_AddGraphics(obj, NULL);
	gfx->txSrc = (SDL_Rect){ 3 * TILE_WIDTH, 0, TILE_WIDTH, TILE_WIDTH };
	gfx->txCenter = (Vec2F){ 0.0, 6.5 };

	ComponentDefense* def = Object_AddDefense(obj, NULL);
	ComponentDefense_CopyExceptSuper(def, &CurrentCharacter()->defense);
	def->hp = def->maxHp;

	ComponentLightSource* light = Object_AddLightSource(obj, 4.0f, NULL);
	light->power = 3.0f;

	Object_AddPrePhysicsStopwatches(obj, STOPWATCH_COUNT);

	return 0;
}
