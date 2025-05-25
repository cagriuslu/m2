#include <pinball/Objects.h>
#include <m2/Game.h>

void BallLauncherSensorOnCollision(m2::Physique&, m2::Physique&, const m2::box2d::Contact&) {
	M2G_PROXY.isOnBallLauncher = true;
}
void BallLauncherSensorOffCollision(m2::Physique&, m2::Physique&) {
	M2G_PROXY.isOnBallLauncher = false;
}
