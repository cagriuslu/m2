#include "m2/component/Physique.h"
#include <m2/Game.h>
#include <box2d/b2_contact.h>
#include <m2/Object.h>
#include <m2/box2d/Shape.h>
#include <m2/thirdparty/physics/ColliderCategory.h>

m2::Physique::Physique(const Id ownerId, const VecFE& position) : Component(ownerId), position(position) {}

std::optional<m2g::pb::PhysicsLayer> m2::Physique::GetCurrentPhysicsLayer() const {
	for (int i = 0; i < PHYSICS_LAYER_COUNT; ++i) {
		if (auto* b = std::get_if<Body>(&body[i]); b && b->IsEnabled()) {
			return static_cast<m2g::pb::PhysicsLayer>(i);
		}
	}
	return std::nullopt;
}

void m2::Physique::DefaultBeginContactCallback(b2Contact& b2_contact) {
	box2d::Contact contact{b2_contact};

	Id physique_id_a = b2_contact.GetFixtureA()->GetBody()->GetUserData().pointer;
	Id physique_id_b = b2_contact.GetFixtureB()->GetBody()->GetUserData().pointer;
	auto& phy_a = M2_LEVEL.physics[physique_id_a];
	auto& phy_b = M2_LEVEL.physics[physique_id_b];
	if (phy_a.onCollision) {
		phy_a.onCollision(phy_a, phy_b, contact);
	}
	if (phy_b.onCollision) {
		phy_b.onCollision(phy_b, phy_a, contact);
	}
}
void m2::Physique::DefaultEndContactCallback(b2Contact& b2_contact) {
	Id physique_id_a = b2_contact.GetFixtureA()->GetBody()->GetUserData().pointer;
	Id physique_id_b = b2_contact.GetFixtureB()->GetBody()->GetUserData().pointer;
	auto& phy_a = M2_LEVEL.physics[physique_id_a];
	auto& phy_b = M2_LEVEL.physics[physique_id_b];
	if (phy_a.offCollision) {
		phy_a.offCollision(phy_a, phy_b);
	}
	if (phy_b.offCollision) {
		phy_b.offCollision(phy_b, phy_a);
	}
}

float m2::CalculateLimitedForce(float curr_speed, float speed_limit) {
	return logf(abs(speed_limit) - abs(curr_speed) + 1.0f); // ln(1) = 0
}
