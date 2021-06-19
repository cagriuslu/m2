#include "../Component.h"
#include "../Object.h"
#include "../Main.h"
#include "../Error.h"
#include <stdio.h>
#include <string.h>

int PhysicsComponentInit(PhysicsComponent* phy, uint64_t objectId) {
	memset(phy, 0, sizeof(PhysicsComponent));
	PROPAGATE_ERROR(ComponentInit((Component*)phy, objectId));
	return 0;
}

void PhysicsComponentDeinit(PhysicsComponent* phy) {
	if (phy->body) {
		Box2DWorldDestroyBody(CurrentLevel()->world, phy->body);
	}
	ComponentDeinit((Component*)phy);
	memset(phy, 0, sizeof(PhysicsComponent));
}

void PhysicsComponentContactCB(Box2DContact* contact) {
	Level* level = CurrentLevel();
	uint64_t phyIdA = (uint64_t) ((uintptr_t) Box2DBodyGetUserData(Box2DFixtureGetBody(Box2DContactGetFixtureA(contact))));
	uint64_t phyIdB = (uint64_t) ((uintptr_t) Box2DBodyGetUserData(Box2DFixtureGetBody(Box2DContactGetFixtureB(contact))));
	PhysicsComponent* phyA = BucketGetById(&level->physics, phyIdA);
	PhysicsComponent* phyB = BucketGetById(&level->physics, phyIdB);
	if (phyA && phyB) {
		if (phyA->onCollision) {
			phyA->onCollision(phyA, phyB);
		}
		if (phyB->onCollision) {
			phyB->onCollision(phyB, phyA);
		}
	}
}
