#include <pinball/Objects.h>
#include <m2/Game.h>
#include <m2/Log.h>

void HeaterSwitchSensorOnCollision(m2::Physique&, m2::Physique&, const m2::box2d::Contact&) {
	LOG_INFO("Flipping the heater switch");
	M2G_PROXY.MutableSimulationInputs().set_heat(!M2G_PROXY.SimulationInputs().heat());
}
