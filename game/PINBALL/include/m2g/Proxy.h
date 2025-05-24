#pragma once
#include <Simulation.pb.h>
#include <m2/sdl/Detail.h>
#include <m2/Proxy.h>

namespace m2g {
	class Proxy : public m2::Proxy {
		std::optional<m2::sdl::ticks_t> _lastSimulationRunTicks;
		std::optional<pinball::pb::SimulationState> _lastSimulationState;
		pinball::pb::SimulationInputs _simulationInputs;

	public:
		const std::string game_identifier = "PINBALL";
		const std::string game_friendly_name = "Pin Ball Evolution";
		/// This aspect ratio divides the screen into 3 equal columns, thus the HUDs are also 16x27.
		const int gameAspectRatioMul = 16;
		const int gameAspectRatioDiv = 27;
		const m2::VecF gravity{0.0f, 24.0f};
		const std::vector<m2::ObjectBlueprint> objectBlueprints = {
			{.objectType = pb::BALL, .defaultSpriteType = pb::SPRITE_BASIC_BALL},
			{.objectType = pb::FLIPPER_LEFT, .defaultSpriteType = pb::SPRITE_BASIC_FLIPPER_LEFT},
			{.objectType = pb::FLIPPER_RIGHT, .defaultSpriteType = pb::SPRITE_BASIC_FLIPPER_RIGHT},
			{.objectType = pb::WALLS, .defaultSpriteType = pb::SPRITE_BASIC_WALLS},
			{.objectType = pb::PLATFORM, .defaultSpriteType = pb::SPRITE_BASIC_PLATFORM},
			{.objectType = pb::BALL_LAUNCHER_SENSOR, .defaultSpriteType = pb::SPRITE_BALL_LAUNCHER_SENSOR},
			{.objectType = pb::PLATFORM_ENTRY_SENSOR, .defaultSpriteType = pb::SPRITE_BALL_PLATFORM_ENTRY_SENSOR},
			{.objectType = pb::PLATFORM_EXIT_SENSOR, .defaultSpriteType = pb::SPRITE_BALL_PLATFORM_EXIT_SENSOR},
			{.objectType = pb::BUMPER_SENSOR, .defaultSpriteType = pb::SPRITE_BUMPER_SENSOR},
		};

		const m2::UiPanelBlueprint* MainMenuBlueprint();
		const m2::UiPanelBlueprint* PauseMenuBlueprint();
		const m2::UiPanelBlueprint* LeftHudBlueprint();
		const m2::UiPanelBlueprint* RightHudBlueprint();

		m2::void_expected LoadForegroundObjectFromLevelBlueprint(m2::Object& obj);

		void OnPostStep();

		// Custom properties

		bool isOnBallLauncher{};

		// Accessors

		const pinball::pb::SimulationState& SimulationState() const { return *_lastSimulationState; }
		const pinball::pb::SimulationInputs& SimulationInputs() const { return _simulationInputs; }

		// Modifiers

		pinball::pb::SimulationInputs& MutableSimulationInputs() { return _simulationInputs; }
	};
}  // namespace m2g
