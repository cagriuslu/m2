#include "../Component.h"
#include "../Object.h"
#include "../Game.h"
#include "../Def.h"

int ComponentPhysique_Init(ComponentPhysique* phy, ID objectId) {
	memset(phy, 0, sizeof(ComponentPhysique));
	XERR_REFLECT(Component_Init((Component*)phy, objectId));
	return 0;
}

void ComponentPhysique_Term(ComponentPhysique* phy) {
	if (phy->body) {
		Box2DWorldDestroyBody(GAME->world, phy->body);
	}
	Component_Term((Component*)phy);
	memset(phy, 0, sizeof(ComponentPhysique));
}

void ComponentPhysique_ContactCB(Box2DContact* contact) {
	ID phyIdA = (ID) ((uintptr_t) Box2DBodyGetUserData(Box2DFixtureGetBody(Box2DContactGetFixtureA(contact))));
	ID phyIdB = (ID) ((uintptr_t) Box2DBodyGetUserData(Box2DFixtureGetBody(Box2DContactGetFixtureB(contact))));
	ComponentPhysique* phyA = Pool_GetById(&GAME->physics, phyIdA);
	ComponentPhysique* phyB = Pool_GetById(&GAME->physics, phyIdB);
	if (phyA && phyB) {
		if (phyA->onCollision) {
			phyA->onCollision(phyA, phyB);
		}
		if (phyB->onCollision) {
			phyB->onCollision(phyB, phyA);
		}
	}
}
