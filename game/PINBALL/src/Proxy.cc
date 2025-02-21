#include <m2g/Proxy.h>
#include <m2/Game.h>
#include <pinball/objects/Player.h>
#include <pinball/objects/Ball.h>
#include <pinball/objects/Flipper.h>

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
						M2_GAME.LoadSinglePlayer(M2_GAME.levels_dir / "001.json", "001");
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
				.name = "LightStatus",
				.x = 0, .y = 0, .w = 16, .h = 3,
				.variant = m2::widget::TextBlueprint{ .text = "Light: OFF" }
			},
			m2::UiWidgetBlueprint{
				.name = "HeaterStatus",
				.x = 0, .y = 3, .w = 16, .h = 3,
				.variant = m2::widget::TextBlueprint{ .text = "Heater: OFF" }
			},
			m2::UiWidgetBlueprint{
				.name = "Temperature",
				.x = 0, .y = 6, .w = 16, .h = 3,
				.variant = m2::widget::TextBlueprint{ .text = "Temperature" }
			},
			m2::UiWidgetBlueprint{
				.name = "WaterStatus",
				.x = 0, .y = 9, .w = 16, .h = 3,
				.variant = m2::widget::TextBlueprint{ .text = "Water" }
			},
			m2::UiWidgetBlueprint{
				.name = "PlantMass",
				.x = 0, .y = 12, .w = 16, .h = 3,
				.variant = m2::widget::TextBlueprint{ .text = "Plant Mass" }
			},
			m2::UiWidgetBlueprint{
				.name = "NutrientAmount",
				.x = 0, .y = 15, .w = 16, .h = 3,
				.variant = m2::widget::TextBlueprint{ .text = "Nutrient Amount" }
			},
			m2::UiWidgetBlueprint{
				.name = "PhotosynthesisRate",
				.x = 0, .y = 18, .w = 16, .h = 3,
				.variant = m2::widget::TextBlueprint{ .text = "Photosynthesis" }
			}
		}
	};
	const m2::UiPanelBlueprint rightHudBlueprint = {
		.name = "RightHud",
		.w = 16, .h = 27,
		.border_width = 0.0001f,
		.widgets = {
			m2::UiWidgetBlueprint{
				.name = "HerbivoreCount",
				.x = 0, .y = 0, .w = 16, .h = 3,
				.variant = m2::widget::TextBlueprint{ .text = "Herbivore Count" }
			},
			m2::UiWidgetBlueprint{
				.name = "HerbivoreSatisfaction",
				.x = 0, .y = 3, .w = 16, .h = 3,
				.variant = m2::widget::TextBlueprint{ .text = "Herbivore Satisfaction" }
			},
			m2::UiWidgetBlueprint{
				.name = "CarnivoreCount",
				.x = 0, .y = 6, .w = 16, .h = 3,
				.variant = m2::widget::TextBlueprint{ .text = "Carnivore Count" }
			},
			m2::UiWidgetBlueprint{
				.name = "CarnivoreSatisfaction",
				.x = 0, .y = 9, .w = 16, .h = 3,
				.variant = m2::widget::TextBlueprint{ .text = "Carnivore Satisfaction" }
			},
			m2::UiWidgetBlueprint{
				.name = "BacteriaMass",
				.x = 0, .y = 12, .w = 16, .h = 3,
				.variant = m2::widget::TextBlueprint{ .text = "Bacteria Mass" }
			},
			m2::UiWidgetBlueprint{
				.name = "WasteAmount",
				.x = 0, .y = 15, .w = 16, .h = 3,
				.variant = m2::widget::TextBlueprint{ .text = "Waste Amount" }
			},
			m2::UiWidgetBlueprint{
				.name = "DecompositionRate",
				.x = 0, .y = 18, .w = 16, .h = 3,
				.variant = m2::widget::TextBlueprint{ .text = "Decomposition" }
			}
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
	switch (obj.object_type()) {
		case pb::ObjectType::PLAYER:
			return LoadPlayer(obj);
		case pb::ObjectType::BALL:
			return LoadBall(obj);
		case pb::ObjectType::FLIPPER_LEFT:
			return LoadFlipper(obj, false);
		case pb::ObjectType::FLIPPER_RIGHT:
			return LoadFlipper(obj, true);
		default:
			throw M2_ERROR("Missing loader for type: " + m2::ToString(obj.object_type()));
	}
}
