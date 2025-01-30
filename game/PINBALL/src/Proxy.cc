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
		.border_width = 0.0f,
	};
	const m2::UiPanelBlueprint rightHudBlueprint = {
		.name = "RightHud",
		.border_width = 0.0f,};
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
