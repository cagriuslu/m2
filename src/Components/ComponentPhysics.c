#include "../Component.h"
#include "../Object.h"
#include "../Main.h"
#include "../Error.h"
#include <stdio.h>
#include <string.h>

int PhysicsComponent_Init(ComponentPhysics* phy, ID objectId) {
	memset(phy, 0, sizeof(ComponentPhysics));
	REFLECT_ERROR(Component_Init((Component*)phy, objectId));
	return 0;
}

void PhysicsComponent_Term(ComponentPhysics* phy) {
	if (phy->body) {
		Box2DWorldDestroyBody(GAME->world, phy->body);
	}
	Component_Term((Component*)phy);
	memset(phy, 0, sizeof(ComponentPhysics));
}

void PhysicsComponent_ContactCB(Box2DContact* contact) {
	ID phyIdA = (ID) ((uintptr_t) Box2DBodyGetUserData(Box2DFixtureGetBody(Box2DContactGetFixtureA(contact))));
	ID phyIdB = (ID) ((uintptr_t) Box2DBodyGetUserData(Box2DFixtureGetBody(Box2DContactGetFixtureB(contact))));
	ComponentPhysics* phyA = Pool_GetById(&GAME->physics, phyIdA);
	ComponentPhysics* phyB = Pool_GetById(&GAME->physics, phyIdB);
	if (phyA && phyB) {
		if (phyA->onCollision) {
			phyA->onCollision(phyA, phyB);
		}
		if (phyB->onCollision) {
			phyB->onCollision(phyB, phyA);
		}
	}
}
