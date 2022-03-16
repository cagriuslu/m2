#include <m2/Component.h>
#include <m2/Game.h>
#include <m2/Def.h>

M2Err ComponentPhysique_Init(ComponentPhysique* phy, ID objectId) {
	memset(phy, 0, sizeof(ComponentPhysique));
	return Component_Init((Component*)phy, objectId);
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
	ComponentPhysique* phyA = Pool_GetById(&GAME->physics, phyIdA); M2ASSERT(phyA);
	ComponentPhysique* phyB = Pool_GetById(&GAME->physics, phyIdB); M2ASSERT(phyB);
	if (phyA->onCollision) { phyA->onCollision(phyA, phyB); }
	if (phyB->onCollision) { phyB->onCollision(phyB, phyA); }
}
