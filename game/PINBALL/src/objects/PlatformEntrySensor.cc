#include <pinball/Objects.h>
#include <m2/Game.h>
#include <m2/Log.h>

void PlatformEntrySensorOnCollision(m2::Physique&, m2::Physique& ball, const m2::box2d::Contact&) {
	LOG_INFO("Moving ball to platform level");
	M2_DEFER(m2::CreateLayerMover(ball.OwnerId(), m2::pb::PhysicsLayer::ABOVE_GROUND, m2::pb::UprightGraphicsLayer::AIRBORNE_UPRIGHT));
}
