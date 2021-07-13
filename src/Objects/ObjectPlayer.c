#include "../Object.h"
#include "../Main.h"
#include "../Box2DUtils.h"
#include "../Controls.h"
#include "../Event.h"
#include <stdio.h>

// Shoot with mouse primary and secondary
// Change primary skill with wheel
// Change secondary skill with middle button
// Shift button is also used during combat
// Ctrl and Alt should not be used during combat

#define STOPWATCH_IDX_RANGED_ATTACK (0)
#define STOPWATCH_IDX_MELEE_ATTACK (1)
#define STOPWATCH_COUNT (2)

static void Player_prePhysics(EventListenerComponent* el) {
	Object* obj = Bucket_GetById(&CurrentLevel()->objects, el->super.objId);
	PhysicsComponent* phy = Bucket_GetById(&CurrentLevel()->physics, obj->physics);
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
		Box2DBodyApplyForceToCenter(phy->body, Vec2FMul(Vec2FNormalize(moveDirection), DeltaTicks() * 25.0f), true);
	}

	Array* stopwatches = Bucket_GetById(&CurrentLevel()->prePhysicsStopwatches, obj->prePhysicsStopwatches);
	unsigned* rangedAttackStopwatch = Array_Get(stopwatches, STOPWATCH_IDX_RANGED_ATTACK);
	if (IsButtonDown(BUTTON_PRIMARY) && (100 < *rangedAttackStopwatch)) {
		Vec2F pointerPosInWorld = CurrentPointerPositionInWorld();
		Vec2F bulletDir = Vec2FSub(pointerPosInWorld, obj->position);

		Object* bullet = Bucket_Mark(&CurrentLevel()->objects, NULL, NULL);
		ComponentOffense* bulletOffense = NULL;
		ObjectBulletInit(bullet, obj->position, bulletDir, &bulletOffense);
		if (bulletOffense) {
			ComponentOffenseCopyExceptSuper(bulletOffense, &CurrentCharacter()->projectileOffense);
		}
		*rangedAttackStopwatch = 0;
	}
	
	unsigned* meleeAttackStopwatch = Array_Get(stopwatches, STOPWATCH_IDX_MELEE_ATTACK);
	if (IsButtonDown(BUTTON_SECONDARY) && (333 < *meleeAttackStopwatch)) {
		Vec2F pointerPosInWorld = CurrentPointerPositionInWorld();
		Vec2F swordDir = Vec2FSub(pointerPosInWorld, obj->position);

		Object* sword = Bucket_Mark(&CurrentLevel()->objects, NULL, NULL);
		ObjectSwordInit(sword, obj->position, &CurrentCharacter()->meleeOffense, swordDir, 150);
		*meleeAttackStopwatch = 0;
	}
}

int ObjectPlayerInit(Object* obj) {
	PROPAGATE_ERROR(ObjectInit(obj, (Vec2F) { 0.0f, 0.0f }));
	ID objId = Bucket_GetId(&CurrentLevel()->objects, obj);
	// Write-back originator ID of Character Offenses
	CurrentCharacter()->charOffense.originator = objId;
	Character_Preprocess(CurrentCharacter());

	EventListenerComponent* el = ObjectAddEventListener(obj, NULL);
	el->prePhysics = Player_prePhysics;

	ID phyId = 0;
	PhysicsComponent* phy = ObjectAddPhysics(obj, &phyId);
	phy->body = Box2DUtils_CreateDynamicDisk(
		phyId,
		obj->position,
		false, // allowSleep
		CATEGORY_PLAYER,
		0.229167f, // Radius
		4.0f, // Mass
		10.0f // Damping
	);

	GraphicsComponent* gfx = ObjectAddGraphics(obj, NULL);
	gfx->txSrc = (SDL_Rect){ 3 * TILE_WIDTH, 0, TILE_WIDTH, TILE_WIDTH };
	gfx->txCenter = (Vec2F){ 0.0, 6.5 };

	ComponentDefense* def = ObjectAddDefense(obj, NULL);
	ComponentDefenseCopyExceptSuper(def, &CurrentCharacter()->defense);
	def->hp = def->maxHp;

	ComponentLightSource* light = ObjectAddLightSource(obj, 4.0f, NULL);
	light->power = 3.0f;

	ObjectAddPrePhysicsStopwatches(obj, STOPWATCH_COUNT);

	return 0;
}
