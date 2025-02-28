#pragma once
#include <Simulation.pb.h>
#include <m2/sdl/Detail.h>
#include <m2/Proxy.h>

namespace m2g {
	class Proxy : public m2::Proxy {
		std::optional<m2::sdl::ticks_t> _lastSimulationRunTicks;
		std::optional<pinball::pb::SimulationState> _lastSimulationState;

	public:
		const std::string game_identifier = "PINBALL";
		const std::string game_friendly_name = "Pin Ball Evolution";
		/// This aspect ratio divides the screen into 3 equal columns, thus the HUDs are also 16x27.
		const int gameAspectRatioMul = 16;
		const int gameAspectRatioDiv = 27;
		const bool gravity = true;

		const m2::UiPanelBlueprint* MainMenuBlueprint();
		const m2::UiPanelBlueprint* PauseMenuBlueprint();
		const m2::UiPanelBlueprint* LeftHudBlueprint();
		const m2::UiPanelBlueprint* RightHudBlueprint();

		m2::void_expected LoadForegroundObjectFromLevelBlueprint(m2::Object& obj);

		void OnPostStep();
	};
}  // namespace m2g
