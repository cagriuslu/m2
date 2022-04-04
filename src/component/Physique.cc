#include "m2/component/Physique.h"
#include <m2/Game.hh>
#include <b2_contact.h>

m2::component::Physique::Physique(ID object_id) : Component(object_id), body(nullptr), onCollision(nullptr) {}

m2::component::Physique::Physique(Physique&& other) noexcept : Component(other.object_id), body(other.body), onCollision(other.onCollision) {
    other.body = nullptr;
}

m2::component::Physique& m2::component::Physique::operator=(Physique&& other) noexcept {
    std::swap(object_id, other.object_id);
    std::swap(body, other.body);
    std::swap(onCollision, other.onCollision);
    return *this;
}

m2::component::Physique::~Physique() {
	if (body) {
		GAME.world->DestroyBody(body);
	}
}

void m2::component::Physique::contact_cb(b2Contact& contact) {
	ID physique_id_a = contact.GetFixtureA()->GetBody()->GetUserData().pointer;
	ID physique_id_b = contact.GetFixtureB()->GetBody()->GetUserData().pointer;
	auto& phy_a = GAME.physics[physique_id_a];
	auto& phy_b = GAME.physics[physique_id_b];
	if (phy_a.onCollision) {
		phy_a.onCollision(phy_a, phy_b);
	}
	if (phy_b.onCollision) {
		phy_b.onCollision(phy_b, phy_a);
	}
}