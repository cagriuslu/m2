#include "../Object.h"
#include "../Main.h"
#include "../Box2DUtils.h"
#include "../Controls.h"
#include "../Event.h"
#include "../Error.h"
#include <stdio.h>

static void God_prePhysics(ComponentEventListener* el) {
	Object* obj = Pool_GetById(&GAME->objects, el->super.objId);
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
}

int ObjectGod_Init(Object* obj) {
	PROPAGATE_ERROR(Object_Init(obj, (Vec2F) { 0.0f, 0.0f }, false));

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
