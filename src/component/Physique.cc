#include "m2/component/Physique.h"
#include <m2/Game.h>
#include <box2d/b2_contact.h>
#include <m2/Object.h>
#include <m2/box2d/Shape.h>
#include <m2/thirdparty/physics/ColliderCategory.h>

m2::Physique::Physique(const Id ownerId, const VecFE& position) : Component(ownerId) {
	SetPosition(position);
}

m2::VecFE m2::Physique::GetPosition() const {
	return std::visit([](const auto& b) -> VecFE { return b.GetPosition(); }, body);
}
m2::FE m2::Physique::GetOrientation() const {
	return std::visit([](const auto& b) -> FE { return b.GetAngle(); }, body);
}
void m2::Physique::SetPosition(const VecFE& position) {
	std::visit([&](auto& b) { b.SetPosition(position); }, body);
}
void m2::Physique::SetOrientation(const FE& orientation) {
	std::visit([&](auto& b) { b.SetAngle(orientation); }, body);
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
