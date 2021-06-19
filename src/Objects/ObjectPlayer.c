#include "../Object.h"
#include "../Main.h"
#include "../Box2DUtils.h"
#include "../Controls.h"
#include "../Event.h"
#include "../Debug.h"
#include <stdio.h>

// Shoot with mouse primary and secondary
// Change primary skill with wheel
// Change secondary skill with middle button
// Shift button is also used during combat
// Ctrl and Alt should not be used during combat

static void Player_prePhysics(EventListenerComponent* el) {
	Level* level = CurrentLevel();
	Object* obj = BucketGetById(&level->objects, el->super.objId);
	if (obj) {
		PhysicsComponent* phy = BucketGetById(&level->physics, obj->physics);
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
		if (IsButtonPressed(BUTTON_PRIMARY)) {
			Vec2F pointerPosInWorld = CurrentPointerPositionInWorld();
			Vec2F bulletDir = Vec2FSub(pointerPosInWorld, obj->position);

			Object* bullet = BucketMark(&level->objects, NULL, NULL);
			ComponentOffense* bulletOffense = NULL;
			ObjectBulletInit(bullet, obj->position, bulletDir, &bulletOffense);
			if (bulletOffense) {
				ComponentOffense* playerOffense = BucketGetById(&level->offenses, obj->offense);
				if (playerOffense) {
					ComponentOffenseCopyExceptSuper(bulletOffense, playerOffense);
				}
			}
		}
	}
}

int ObjectPlayerInit(Object* obj) {
	PROPAGATE_ERROR(ObjectInit(obj, (Vec2F) { 0.0f, 0.0f }));
	uint64_t objId = BucketGetId(&CurrentLevel()->objects, obj);

	EventListenerComponent* el = ObjectAddAndInitEventListener(obj, NULL);
	el->prePhysics = Player_prePhysics;

	uint64_t phyId = 0;
	PhysicsComponent* phy = ObjectAddAndInitPhysics(obj, &phyId);
	phy->body = Box2DUtilsCreateDynamicDisk(
		phyId,
		obj->position,
		DONT_SLEEP,
		PLAYER_CATEGORY,
		0.229167f, // Radius
		4.0f, // Mass
		10.0f // Damping
	);

	GraphicsComponent* gfx = ObjectAddAndInitGraphics(obj, NULL);
	gfx->txSrc = (SDL_Rect){ 3 * TILE_WIDTH, 0, TILE_WIDTH, TILE_WIDTH };
	gfx->txOffset = (Vec2F){ 0.0, -6.5 };

	ComponentDefense* def = ObjectAddAndInitDefense(obj, NULL);
	def->hp = 100;
	def->maxHp = 100;

	ComponentOffense* off = ObjectAddAndInitOffense(obj, NULL);
	off->hp = 15;
	off->originator = objId;
	off->ticksLeft = 750;

	return 0;
}
