#include <pinball/Objects.h>
#include <m2/Game.h>
#include <m2/Log.h>

void WaterSprinklerSensorOnCollision(m2::Physique&, m2::Physique& ball, const m2::box2d::Contact&) {
	// Look up the speed of the ball
	const auto lengthSquared = std::get<m2::Physique::Body>(ball.body[m2::I(m2g::pb::PhysicsLayer::PHYSICS_DEFAULT_LAYER)]).GetLinearVelocity().GetLengthSquared();
	const auto waterAmount = std::clamp(lengthSquared / 10.0f, 1.0f, 25.0f);
	LOG_INFO("Adding water", waterAmount);
	M2G_PROXY.MutableSimulationInputs().set_extra_water(waterAmount);
}
