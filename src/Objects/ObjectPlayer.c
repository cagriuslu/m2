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

static void Player_prePhysics(EventListenerComponent* el) {
	Level* level = CurrentLevel();
	//fprintf(stderr, "NewPlayer_prePhysics called for %u\n", el->super.object);
	Object* obj = BucketGetById(&level->objects, el->super.object);
	if (obj) {
		PhysicsComponent* phy = BucketGetById(&level->physics, obj->physics);
		if (phy && phy->body) {
			if (IsKeyDown(KEY_UP)) {
				Box2DBodyApplyForceToCenter(phy->body, (Vec2F) { 0.0, -100.0 }, true);
			}
			if (IsKeyDown(KEY_DOWN)) {
				Box2DBodyApplyForceToCenter(phy->body, (Vec2F) { 0.0, 100.0 }, true);
			}
			if (IsKeyDown(KEY_LEFT)) {
				Box2DBodyApplyForceToCenter(phy->body, (Vec2F) { -100.0, 0.0 }, true);
			}
			if (IsKeyDown(KEY_RIGHT)) {
				Box2DBodyApplyForceToCenter(phy->body, (Vec2F) { 100.0, 0.0 }, true);
			}
		}
		if (IsButtonPressed(BUTTON_PRIMARY)) {
			DebugVec2F("pointer in world", CurrentPointerPositionInWorld());
			Vec2F pointerPosInWorld = CurrentPointerPositionInWorld();
			Vec2F bulletDir = Vec2FSub(pointerPosInWorld, obj->position);

			Object* bullet = BucketMark(&level->objects, NULL, NULL);
			ObjectBulletInit(bullet, obj->position, bulletDir);
		}
	}
}

int ObjectPlayerInit(Object* obj) {
	PROPAGATE_ERROR(ObjectInit(obj, (Vec2F) { 0.0f, 0.0f }));

	EventListenerComponent* el = ObjectAddAndInitEventListener(obj, NULL);
	el->prePhysics = Player_prePhysics;

	uint32_t phyId = 0;
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

	return 0;
}
