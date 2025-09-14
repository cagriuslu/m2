#include <pinball/Objects.h>
#include <m2/Game.h>
#include <pinball/Pinball.h>

m2::void_expected LoadPlayer(m2::Object& obj) {
	// Set player handle
	M2_LEVEL.playerId = obj.GetId();
	// Adjust game scale
	M2_GAME.SetGameHeightM(gLevelDimensions.GetY());
	// Load edges of the screen
	LoadEdge();

	auto& phy = obj.AddPhysique();
	phy.position = gLevelCenter; // Put the player on the center
	phy.preStep = [](MAYBE m2::Physique& phy, const m2::Stopwatch::Duration&) {
		if (M2_GAME.events.PopKeyPress(m2g::pb::TOGGLE_LIGHT)) {
			M2G_PROXY.MutableSimulationInputs().set_light(!M2G_PROXY.SimulationInputs().light());
		}
		if (M2_GAME.events.PopKeyPress(m2g::pb::ADD_WATER)) {
			M2G_PROXY.MutableSimulationInputs().set_extra_water(5.0f);
		}
		if (M2_GAME.events.PopKeyPress(m2g::pb::TOGGLE_HEATER)) {
			M2G_PROXY.MutableSimulationInputs().set_heat(!M2G_PROXY.SimulationInputs().heat());
		}
	};

	return {};
}
