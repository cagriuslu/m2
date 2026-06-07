#include <m2/physics/StaticBody.h>
#include <m2/M2.h>

using namespace m2;
using namespace m2::physics;

VecF StaticBody::GetPosition() const { return static_cast<VecF>(position); }
FE StaticBody::GetAngle() const { return orientation; }
VecF StaticBody::GetLinearVelocity() const { throw M2_ERROR("Not implemented for static bodies"); }
FE StaticBody::GetAngularVelocity() const { throw M2_ERROR("Not implemented for static bodies"); }

void StaticBody::SetEnabled(const bool enabled) { _enabled = enabled; }
void StaticBody::SetPosition(const VecFE& position_) { position = position_; }
void StaticBody::SetAngle(const FE angle) { orientation = angle; }
void StaticBody::SetLinearVelocity(const VecFE&) { throw M2_ERROR("Not implemented for static bodies"); }
void StaticBody::SetAngularVelocity(FE) { throw M2_ERROR("Not implemented for static bodies"); }
void StaticBody::ApplyForceToCenter(const VecFE&) { throw M2_ERROR("Not implemented for static bodies"); }
void StaticBody::TeleportToAnother(const StaticBody& other) {
    position = other.position;
    orientation = other.orientation;
    _enabled = other._enabled;
}
