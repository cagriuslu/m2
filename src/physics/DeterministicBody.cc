#include <m2/physics/DeterministicBody.h>
#include <m2/M2.h>

using namespace m2;
using namespace m2::physics;

DeterministicBody DeterministicBody::CreateFromDefinition(const thirdparty::physics::RigidBodyDefinition&, Id, const VecF&, float) {
	throw M2_ERROR("Not implemented for deterministic games");
}

VecFE DeterministicBody::GetLinearVelocity() const { throw M2_ERROR("Not implemented for deterministic games"); }
FE DeterministicBody::GetAngularVelocity() const { throw M2_ERROR("Not implemented for deterministic games"); }

void DeterministicBody::SetEnabled(bool) { throw M2_ERROR("Not implemented for deterministic games"); }
void DeterministicBody::SetLinearVelocity(const VecFE&) { throw M2_ERROR("Not implemented for deterministic games"); }
void DeterministicBody::SetAngularVelocity(FE) { throw M2_ERROR("Not implemented for deterministic games"); }
void DeterministicBody::ApplyForceToCenter(const VecFE&) { throw M2_ERROR("Not implemented for deterministic games"); }
void DeterministicBody::TeleportToAnother(const DeterministicBody&) { throw M2_ERROR("Not implemented for deterministic games"); }
