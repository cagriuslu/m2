#include <m2/physics/DeterministicBody.h>

using namespace m2;
using namespace m2::physics;

DeterministicBody DeterministicBody::CreateFromDefinition(const thirdparty::physics::RigidBodyDefinition&, Id, const VecF&, float, m2g::pb::PhysicsLayer) {
	throw M2_ERROR("Not implemented for deterministic games");
}

VecFE DeterministicBody::GetPosition() const {
	throw M2_ERROR("Not implemented yet");
}
FE DeterministicBody::GetAngle() const {
	throw M2_ERROR("Not implemented yet");
}

void DeterministicBody::SetEnabled(bool) {
	throw M2_ERROR("Not implemented for deterministic games");
}
void DeterministicBody::TeleportToAnother(const DeterministicBody&) {
	throw M2_ERROR("Not implemented for deterministic games");
}
