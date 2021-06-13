#include "PhysicsComponent.h"
#include "Main.h"
#include "Error.h"
#include <stdio.h>
#include <string.h>

int PhysicsComponentInit(PhysicsComponent* phy, uint32_t objectId) {
	memset(phy, 0, sizeof(PhysicsComponent));
	PROPAGATE_ERROR(ComponentInit((Component*)phy, objectId));
	return 0;
}

void PhysicsComponentDeinit(PhysicsComponent* phy) {
	if (phy->body) {
		Box2DWorldDestroyBody(CurrentLevel()->world, phy->body);
	}
	ComponentDeinit((Component*)phy);
}

void PhysicsComponentContactCB(Box2DContact* contact) {
	// TODO
	/*
	fprintf(stderr, "Contact begin\n");

	Object* a = Box2DBodyGetUserData(Box2DFixtureGetBody(Box2DContactGetFixtureA(contact)));
	Object* b = Box2DBodyGetUserData(Box2DFixtureGetBody(Box2DContactGetFixtureB(contact)));

	if (IS_OBJECT(a->type)) {
		GameObject* aGO = (GameObject*)a;
		if (aGO->onCollision) {
			aGO->onCollision(aGO, b);
		}
	}

	if (IS_OBJECT(b->type)) {
		GameObject* bGO = (GameObject*)b;
		if (bGO->onCollision) {
			bGO->onCollision(bGO, a);
		}
	}
	*/
}
