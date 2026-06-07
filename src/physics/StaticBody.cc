#include <m2/physics/StaticBody.h>
#include <m2/M2.h>

using namespace m2;
using namespace m2::physics;

VecFE StaticBody::GetPosition() const { return position; }
FE StaticBody::GetAngle() const { return orientation; }
VecFE StaticBody::GetLinearVelocity() const { throw M2_ERROR("Not implemented for static bodies"); }
FE StaticBody::GetAngularVelocity() const { throw M2_ERROR("Not implemented for static bodies"); }

void StaticBody::SetPosition(const VecFE& position_) { position = position_; }
void StaticBody::SetAngle(const FE angle) { orientation = angle; }
void StaticBody::SetLinearVelocity(const VecFE&) { throw M2_ERROR("Not implemented for static bodies"); }
void StaticBody::SetAngularVelocity(FE) { throw M2_ERROR("Not implemented for static bodies"); }
void StaticBody::ApplyForceToCenter(const VecFE&) { throw M2_ERROR("Not implemented for static bodies"); }
void StaticBody::TeleportToAnother(const StaticBody& other) {
    position = other.position;
    orientation = other.orientation;
}
