#include "../Component.h"
#include "../Object.h"
#include "../Main.h"
#include "../Error.h"
#include <stdio.h>
#include <string.h>

int PhysicsComponent_Init(ComponentPhysics* phy, ID objectId) {
	memset(phy, 0, sizeof(ComponentPhysics));
	PROPAGATE_ERROR(Component_Init((Component*)phy, objectId));
	return 0;
}

void PhysicsComponent_Term(ComponentPhysics* phy) {
	if (phy->body) {
		Box2DWorldDestroyBody(CurrentLevel()->world, phy->body);
	}
	Component_Term((Component*)phy);
	memset(phy, 0, sizeof(ComponentPhysics));
}

void PhysicsComponent_ContactCB(Box2DContact* contact) {
	Level* level = CurrentLevel();
	ID phyIdA = (ID) ((uintptr_t) Box2DBodyGetUserData(Box2DFixtureGetBody(Box2DContactGetFixtureA(contact))));
	ID phyIdB = (ID) ((uintptr_t) Box2DBodyGetUserData(Box2DFixtureGetBody(Box2DContactGetFixtureB(contact))));
	ComponentPhysics* phyA = Bucket_GetById(&level->physics, phyIdA);
	ComponentPhysics* phyB = Bucket_GetById(&level->physics, phyIdB);
	if (phyA && phyB) {
		if (phyA->onCollision) {
			phyA->onCollision(phyA, phyB);
		}
		if (phyB->onCollision) {
			phyB->onCollision(phyB, phyA);
		}
	}
}
