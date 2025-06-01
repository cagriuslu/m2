#include <pinball/Objects.h>
#include <m2/Game.h>
#include <m2/Log.h>

void PlatformExitSensorOnCollision(m2::Physique&, m2::Physique& ball, const m2::box2d::Contact&) {
	LOG_INFO("Moving ball to ground level");
	M2_DEFER(m2::CreateLayerMover(ball.OwnerId(), m2::PhysicsLayer::P0, m2::ForegroundDrawLayer::F0_BOTTOM));
}
