#include <b2_contact.h>
#include <b2_world.h>

#include "m2/Component.hh"
#include "m2/Game.hh"
#include "m2/Def.hh"

M2Err ComponentPhysique_Init(ComponentPhysique* phy, ID objectId) {
	memset(phy, 0, sizeof(ComponentPhysique));
	return Component_Init((Component*)phy, objectId);
}

void ComponentPhysique_Term(ComponentPhysique* phy) {
	if (phy->body) {
		GAME.world->DestroyBody(phy->body);
	}
	Component_Term((Component*)phy);
	memset(phy, 0, sizeof(ComponentPhysique));
}

void ComponentPhysique_ContactCB(b2Contact* contact) {
	ID physique_id_a = contact->GetFixtureA()->GetBody()->GetUserData().pointer;
	ID physique_id_b = contact->GetFixtureB()->GetBody()->GetUserData().pointer;
	auto& phy_a = GAME.physics[physique_id_a];
	auto& phy_b = GAME.physics[physique_id_b];
	if (phy_a.onCollision) {
		phy_a.onCollision(&phy_a, &phy_b);
	}
	if (phy_b.onCollision) {
		phy_b.onCollision(&phy_b, &phy_a);
	}
}
