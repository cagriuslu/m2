#include "../Object.h"
#include "../Main.h"
#include "../Box2DUtils.h"
#include "../Controls.h"
#include "../Event.h"
#include "../Error.h"
#include <stdio.h>

static void God_prePhysics(EventListenerComponent* el) {
	Object* obj = BucketGetById(&CurrentLevel()->objects, el->super.objId);
	if (obj) {
		PhysicsComponent* phy = BucketGetById(&CurrentLevel()->physics, obj->physics);
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

int ObjectGodInit(Object* obj) {
	PROPAGATE_ERROR(ObjectInit(obj, (Vec2F) { 0.0f, 0.0f }));

	EventListenerComponent* el = ObjectAddAndInitEventListener(obj, NULL);
	el->prePhysics = God_prePhysics;

	uint64_t phyId = 0;
	PhysicsComponent* phy = ObjectAddAndInitPhysics(obj, &phyId);
	phy->body = Box2DUtilsCreateDynamicDisk(
		phyId,
		obj->position,
		DONT_SLEEP,
		0,
		0.229167f, // Radius
		4.0f, // Mass
		10.0f // Damping
	);

	return 0;
}
