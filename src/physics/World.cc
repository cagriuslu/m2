#include <m2/physics/World.h>
#include <m2/Error.h>

const m2::RigidBody& m2::World::GetRigidBody(const size_t index) const {
	if (_rigidBodies.size() <= index) {
		throw M2_ERROR("Rigid body index out of bounds");
	}
	return _rigidBodies[index];
}

m2::RigidBody& m2::World::GetRigidBody(const size_t index) {
	if (_rigidBodies.size() <= index) {
		throw M2_ERROR("Rigid body index out of bounds");
	}
	return _rigidBodies[index];
}
void m2::World::Integrate() {
	for (auto& body : _rigidBodies) {
		body.Integrate();
	}
}
