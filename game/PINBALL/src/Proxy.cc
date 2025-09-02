#include <m2g/Proxy.h>
#include <pinball/Objects.h>
#include <pinball/Simulation.h>
#include <m2/Game.h>
#include <m2/ui/widget/ProgressBar.h>
#include <m2/ui/widget/Text.h>
#include <m2g_ObjectType.pb.h>
#include <numeric>

namespace {
	const m2::UiPanelBlueprint mainMenuBlueprint = {
		.name = "MainMenu",
		.w = 3, .h = 3,
		.border_width = 0.0f,
		.widgets = {
			m2::UiWidgetBlueprint{
				.x = 1, .y = 1, .w = 1, .h = 1,
				.background_color = {35, 35, 35, 255},
				.variant = m2::widget::TextBlueprint{
					.text = "Start",
					.onAction = [](MAYBE const m2::widget::Text& text) -> m2::UiAction {
						M2_GAME.LoadSinglePlayer(M2_GAME.GetResources().GetLevelsDir() / "001.json", "001");
						return m2::MakeReturnAction();
					}
				}
			}
		}
	};
	const m2::UiPanelBlueprint pauseMenuBlueprint = {};
	const m2::UiPanelBlueprint leftHudBlueprint = {
		.name = "LeftHud",
		.w = 16, .h = 27,
		.border_width = 0.0001f,
		.widgets = {
			m2::UiWidgetBlueprint{
				.x = 0, .y = 0, .w = 16, .h = 1,
				.border_width = 0.0f,
				.variant = m2::widget::TextBlueprint{ .text = "Light" }
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 1, .w = 16, .h = 1,
				.variant = m2::widget::TextBlueprint{
					.onUpdate = [](m2::widget::Text& self) -> m2::UiAction {
						self.set_text(M2G_PROXY.SimulationInputs().light() ? "ON" : "OFF");
						return m2::MakeContinueAction();
					}
				}
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 3, .w = 16, .h = 1,
				.border_width = 0.0f,
				.variant = m2::widget::TextBlueprint{ .text = "Heater" }
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 4, .w = 16, .h = 1,
				.variant = m2::widget::TextBlueprint{
					.onUpdate = [](m2::widget::Text& self) -> m2::UiAction {
						self.set_text(M2G_PROXY.SimulationInputs().heat() ? "ON" : "OFF");
						return m2::MakeContinueAction();
					}
				}
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 6, .w = 16, .h = 1,
				.border_width = 0.0f,
				.variant = m2::widget::TextBlueprint{ .text = "Temperature" }
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 7, .w = 16, .h = 1,
				.variant = m2::widget::ProgressBarBlueprint{
					.bar_color = {255, 255, 255, 255},
					.onUpdate = [](m2::widget::ProgressBar& self) {
						self.SetProgress(m2::Normalize(M2G_PROXY.SimulationState().temperature(),
								pinball::MIN_TEMPERATURE, pinball::MAX_TEMPERATURE));
					}
				}
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 9, .w = 16, .h = 1,
				.border_width = 0.0f,
				.variant = m2::widget::TextBlueprint{ .text = "Water" }
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 10, .w = 16, .h = 1,
				.variant = m2::widget::ProgressBarBlueprint{
					.bar_color = {255, 255, 255, 255},
					.onUpdate = [](m2::widget::ProgressBar& self) {
						self.SetProgress(m2::Normalize(M2G_PROXY.SimulationState().water_mass(), 0.0f,
								pinball::MAX_WATER_MASS));
					}
				}
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 12, .w = 16, .h = 1,
				.border_width = 0.0f,
				.variant = m2::widget::TextBlueprint{ .text = "Plant Mass" }
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 13, .w = 16, .h = 1,
				.variant = m2::widget::ProgressBarBlueprint{
					.bar_color = {255, 255, 255, 255},
					.onUpdate = [](m2::widget::ProgressBar& self) {
						self.SetProgress(m2::Normalize(M2G_PROXY.SimulationState().plant_mass(), 0.0f,
								pinball::PLANT_MAX_MASS_KG));
					}
				}
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 15, .w = 16, .h = 1,
				.border_width = 0.0f,
				.variant = m2::widget::TextBlueprint{ .text = "Nutrient Amount" }
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 16, .w = 16, .h = 1,
				.variant = m2::widget::ProgressBarBlueprint{
					.bar_color = {255, 255, 255, 255},
					.onUpdate = [](m2::widget::ProgressBar& self) {
						self.SetProgress(m2::Normalize(M2G_PROXY.SimulationState().nutrient_mass(), 0.0f,
								pinball::MAX_NUTRIENT_MASS));
					}
				}
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 18, .w = 16, .h = 1,
				.border_width = 0.0f,
				.variant = m2::widget::TextBlueprint{ .text = "Photosynthesis" }
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 19, .w = 16, .h = 1,
				.variant = m2::widget::ProgressBarBlueprint{
					.bar_color = {255, 255, 255, 255},
					.onUpdate = [](m2::widget::ProgressBar& self) {
						self.SetProgress(M2G_PROXY.SimulationState().last_photosynthesis_rate());
					}
				}
			},
		}
	};
	const m2::UiPanelBlueprint rightHudBlueprint = {
		.name = "RightHud",
		.w = 16, .h = 27,
		.border_width = 0.0001f,
		.widgets = {
			m2::UiWidgetBlueprint{
				.x = 0, .y = 0, .w = 16, .h = 1,
				.border_width = 0.0f,
				.variant = m2::widget::TextBlueprint{ .text = "Herbivore Count" }
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 1, .w = 16, .h = 1,
				.variant = m2::widget::TextBlueprint{
					.onUpdate = [](m2::widget::Text& self) -> m2::UiAction {
						self.set_text(std::to_string(std::ranges::count_if(M2G_PROXY.SimulationState().animals(),
								[](const auto& animal) { return animal.type() == pinball::pb::Animal_Type_HERBIVORE; })));
						return m2::MakeContinueAction();
					}
				}
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 3, .w = 16, .h = 1,
				.border_width = 0.0f,
				.variant = m2::widget::TextBlueprint{ .text = "Herbivore Satisfaction" }
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 4, .w = 16, .h = 1,
				.variant = m2::widget::ProgressBarBlueprint{
					.bar_color = {255, 255, 255, 255},
					.onUpdate = [](m2::widget::ProgressBar& self) {
						const auto count = std::ranges::count_if(M2G_PROXY.SimulationState().animals(),
								[](const auto& animal) { return animal.type() == pinball::pb::Animal_Type_HERBIVORE; });
						if (0 < count) {
							const auto sumOfHunger = std::accumulate(M2G_PROXY.SimulationState().animals().begin(),
									M2G_PROXY.SimulationState().animals().end(), 0.0f,
											[](const auto sum, const auto& animal) {
										if (animal.type() == pinball::pb::Animal_Type_HERBIVORE) {
											return sum + animal.hunger();
										}
										return sum;
									});
							const auto averageHunger = sumOfHunger / m2::ToFloat(count);
							const auto averageSatisfaction = 1.0f - averageHunger;
							self.SetProgress(averageSatisfaction);
						} else {
							self.SetProgress(0.0f);
						}
					}
				}
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 6, .w = 16, .h = 1,
				.border_width = 0.0f,
				.variant = m2::widget::TextBlueprint{ .text = "Carnivore Count" }
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 7, .w = 16, .h = 1,
				.variant = m2::widget::TextBlueprint{
					.onUpdate = [](m2::widget::Text& self) -> m2::UiAction {
						self.set_text(std::to_string(std::ranges::count_if(M2G_PROXY.SimulationState().animals(),
								[](const auto& animal) { return animal.type() == pinball::pb::Animal_Type_CARNIVORE; })));
						return m2::MakeContinueAction();
					}
				}
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 9, .w = 16, .h = 1,
				.border_width = 0.0f,
				.variant = m2::widget::TextBlueprint{ .text = "Carnivore Satisfaction" }
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 10, .w = 16, .h = 1,
				.variant = m2::widget::ProgressBarBlueprint{
					.bar_color = {255, 255, 255, 255},
					.onUpdate = [](m2::widget::ProgressBar& self) {
						const auto count = std::ranges::count_if(M2G_PROXY.SimulationState().animals(),
								[](const auto& animal) { return animal.type() == pinball::pb::Animal_Type_CARNIVORE; });
						if (0 < count) {
							const auto sumOfHunger = std::accumulate(M2G_PROXY.SimulationState().animals().begin(),
									M2G_PROXY.SimulationState().animals().end(), 0.0f,
											[](const auto sum, const auto& animal) {
										if (animal.type() == pinball::pb::Animal_Type_CARNIVORE) {
											return sum + animal.hunger();
										}
										return sum;
									});
							const auto averageHunger = sumOfHunger / m2::ToFloat(count);
							const auto averageSatisfaction = 1.0f - averageHunger;
							self.SetProgress(averageSatisfaction);
						} else {
							self.SetProgress(0.0f);
						}
					}
				}
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 12, .w = 16, .h = 1,
				.border_width = 0.0f,
				.variant = m2::widget::TextBlueprint{ .text = "Bacteria Mass" }
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 13, .w = 16, .h = 1,
				.variant = m2::widget::ProgressBarBlueprint{
					.bar_color = {255, 255, 255, 255},
					.onUpdate = [](m2::widget::ProgressBar& self) {
						self.SetProgress(m2::Normalize(M2G_PROXY.SimulationState().bacteria_mass(), 0.0f,
								pinball::BACTERIA_MAX_MASS_KG));
					}
				}
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 15, .w = 16, .h = 1,
				.border_width = 0.0f,
				.variant = m2::widget::TextBlueprint{ .text = "Waste Amount" }
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 16, .w = 16, .h = 1,
				.variant = m2::widget::ProgressBarBlueprint{
					.bar_color = {255, 255, 255, 255},
					.onUpdate = [](m2::widget::ProgressBar& self) {
						self.SetProgress(m2::Normalize(M2G_PROXY.SimulationState().waste_mass(), 0.0f,
								pinball::MAX_WASTE_MASS));
					}
				}
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 18, .w = 16, .h = 1,
				.border_width = 0.0f,
				.variant = m2::widget::TextBlueprint{ .text = "Decomposition" }
			},
			m2::UiWidgetBlueprint{
				.x = 0, .y = 19, .w = 16, .h = 1,
				.variant = m2::widget::ProgressBarBlueprint{
					.bar_color = {255, 255, 255, 255},
					.onUpdate = [](m2::widget::ProgressBar& self) {
						self.SetProgress(M2G_PROXY.SimulationState().last_decomposition_rate());
					}
				}
			},
		}
	};
}

const m2::UiPanelBlueprint* m2g::Proxy::MainMenuBlueprint() {
	return &mainMenuBlueprint;
}
const m2::UiPanelBlueprint* m2g::Proxy::PauseMenuBlueprint() {
	return &pauseMenuBlueprint;
}
const m2::UiPanelBlueprint* m2g::Proxy::LeftHudBlueprint() {
	return &leftHudBlueprint;
}
const m2::UiPanelBlueprint* m2g::Proxy::RightHudBlueprint() {
	return &rightHudBlueprint;
}

m2::void_expected m2g::Proxy::LoadForegroundObjectFromLevelBlueprint(m2::Object& obj) {
	switch (obj.GetType()) {
		case pb::ObjectType::PLAYER:
			return LoadPlayer(obj);
		case pb::ObjectType::BALL:
			return LoadBall(obj);
		case pb::ObjectType::FLIPPER_LEFT:
			return LoadFlipper(obj, false);
		case pb::ObjectType::FLIPPER_RIGHT:
			return LoadFlipper(obj, true);
		case pb::ObjectType::WALLS:
			return LoadWall(obj);
		case pb::ObjectType::PLATFORM:
			return LoadPlatform(obj);
		case pb::ObjectType::BALL_LAUNCHER_SENSOR:
			return LoadGenericBallSensor(obj, m2::PhysicsLayer::P0, BallLauncherSensorOnCollision, BallLauncherSensorOffCollision);
		case pb::ObjectType::PLATFORM_ENTRY_SENSOR:
			return LoadGenericBallSensor(obj, m2::PhysicsLayer::P0, PlatformEntrySensorOnCollision, {});
		case pb::ObjectType::PLATFORM_EXIT_SENSOR:
			return LoadGenericBallSensor(obj, m2::PhysicsLayer::P1, PlatformExitSensorOnCollision, {});
		case pb::ObjectType::LIGHT_SWITCH_SENSOR:
			return LoadGenericBallSensor(obj, m2::PhysicsLayer::P0, LightSwitchSensorOnCollision, {});
		case pb::ObjectType::HEATER_SWITCH_SENSOR:
			return LoadGenericBallSensor(obj, m2::PhysicsLayer::P0, HeaterSwitchSensorOnCollision, {});
		case pb::ObjectType::WATER_SPRINKLER_SENSOR:
			return LoadGenericBallSensor(obj, m2::PhysicsLayer::P0, WaterSprinklerSensorOnCollision, {});
		case pb::ObjectType::BUMPER_SENSOR:
			return LoadBumperSensor(obj);
		case pb::ObjectType::DROP_GATE:
			return LoadDropGate(obj);
		case pb::ObjectType::CIRCULAR_BUMPER:
			return LoadCircularBumperSensor(obj);
		case pb::ObjectType::DOOR:
			return LoadDoor(obj);
		default:
			throw M2_ERROR("Missing loader for type: " + m2::ToString(obj.GetType()));
	}
}

void m2g::Proxy::OnPostStep(const m2::Stopwatch::Duration&) {
	// On first run or enough time passed
	if (const auto now = m2::sdl::get_ticks(); not _lastSimulationRunTicks
			|| *_lastSimulationRunTicks + pinball::SIMULATION_TICK_PERIOD_TICKS <= now) {
		// Init simulation state on first run
		if (not _lastSimulationState) {
			_lastSimulationState = pinball::InitialSimulationState(AnimalAllocator);
		}

		_lastSimulationState = pinball::AdvanceSimulation(*_lastSimulationState, SimulationInputs(), AnimalAllocator, AnimalDeallocator);
		// Clear water
		MutableSimulationInputs().clear_extra_water();

		_lastSimulationRunTicks = now;
	}
}
