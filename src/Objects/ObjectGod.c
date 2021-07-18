#include "../Object.h"
#include "../Main.h"
#include "../Box2DUtils.h"
#include "../Controls.h"
#include "../Event.h"
#include "../Error.h"
#include <stdio.h>

static void God_prePhysics(ComponentEventListener* el) {
	Object* obj = Bucket_GetById(&CurrentLevel()->objects, el->super.objId);
	if (obj) {
		ComponentPhysics* phy = Bucket_GetById(&CurrentLevel()->physics, obj->physics);
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
	}
}

int ObjectGod_Init(Object* obj) {
	PROPAGATE_ERROR(Object_Init(obj, (Vec2F) { 0.0f, 0.0f }));

	ComponentEventListener* el = Object_AddEventListener(obj, NULL);
	el->prePhysics = God_prePhysics;

	ID phyId = 0;
	ComponentPhysics* phy = Object_AddPhysics(obj, &phyId);
	phy->body = Box2DUtils_CreateDynamicDisk(
		phyId,
		obj->position,
		false, // allow sleep
		0,
		0.229167f, // Radius
		4.0f, // Mass
		10.0f // Damping
	);

	return 0;
}
