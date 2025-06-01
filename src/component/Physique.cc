#include "m2/component/Physique.h"
#include <m2/Game.h>
#include <box2d/b2_contact.h>
#include <m2/Object.h>
#include <m2/box2d/Shape.h>
#include <m2/third_party/physics/ColliderCategory.h>

m2::Physique::Physique(Id object_id) : Component(object_id) {}

m2::Physique::Physique(Physique&& other) noexcept
		: Component(other._owner_id), preStep(std::move(other.preStep)), postStep(std::move(other.postStep)),
		body(std::move(other.body)), rigidBodyIndex(std::move(other.rigidBodyIndex)),
		onCollision(std::move(other.onCollision)), offCollision(std::move(other.offCollision)) {
	for (auto& b : body) {
		b.reset();
	}
	other.rigidBodyIndex.reset();
}

m2::Physique& m2::Physique::operator=(Physique&& other) noexcept {
    std::swap(_owner_id, other._owner_id);
    std::swap(preStep, other.preStep);
    std::swap(postStep, other.postStep);
    std::swap(body, other.body);
    std::swap(rigidBodyIndex, other.rigidBodyIndex);
	std::swap(onCollision, other.onCollision);
	std::swap(offCollision, other.offCollision);
    return *this;
}

std::optional<m2::PhysicsLayer> m2::Physique::GetCurrentPhysicsLayer() const {
	for (int i = 0; i < gPhysicsLayerCount; ++i) {
		if (body[i] && body[i]->IsEnabled()) {
			return static_cast<PhysicsLayer>(i);
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
