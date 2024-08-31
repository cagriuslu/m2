#include <cuzn/ui/Detail.h>
#include <m2/Level.h>
#include <m2/ui/Action.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextSelection.h>
#include <cuzn/journeys/BuildJourney.h>

using namespace m2;
using namespace m2::ui;
using namespace m2::ui::widget;

std::optional<m2g::pb::ItemType> ask_for_industry_selection(m2g::pb::ItemType industry_1, m2g::pb::ItemType industry_2) {
	LOG_INFO("Asking player to select an industry...");

	auto blueprint = Blueprint{
		.w = 60, .h = 40,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			WidgetBlueprint{
				.x = 57, .y = 0, .w = 3, .h = 3,
				.variant = TextBlueprint{
					.text = "X",
					.on_action = [](MAYBE const Text& self) -> Action {
						return make_return_action();
					}
				}
			},
			WidgetBlueprint{
				.x = 5, .y = 14, .w = 50, .h = 5,
				.variant = TextBlueprint{
					.text = M2_GAME.get_named_item(industry_1).in_game_name(),
					.on_action = [industry_1](MAYBE const Text& self) -> Action {
						return make_return_action<m2g::pb::ItemType>(industry_1);
					}
				}
			},
			WidgetBlueprint{
				.x = 5, .y = 21, .w = 50, .h = 5,
				.variant = TextBlueprint{
					.text = M2_GAME.get_named_item(industry_2).in_game_name(),
					.on_action = [industry_2](MAYBE const Text& self) -> Action {
						return make_return_action<m2g::pb::ItemType>(industry_2);
					}
				}
			}
		}
	};

	std::optional<m2g::pb::ItemType> selected_industry;
	m2::ui::State::create_execute_sync(
		std::make_unique<m2::ui::Blueprint>(blueprint),
		M2_GAME.dimensions().game_and_hud.ratio({0.15f, 0.15f, 0.7f, 0.7f}))
		.if_void_return([&]() {
			LOG_INFO("Industry selection cancelled");
		})
		.if_return<m2g::pb::ItemType>([&](auto industry) {
			LOG_INFO("Industry selected", m2g::pb::ItemType_Name(industry));
			selected_industry = industry;
		});
	return selected_industry;
}

bool ask_for_confirmation(const std::string& question1, const std::string& question2, const std::string& accept_text, const std::string& decline_text) {
	auto blueprint = Blueprint{
		.w = 60, .h = 40,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			WidgetBlueprint{
				.x = 5, .y = 5, .w = 50, .h = 5,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = question1,
					.horizontal_alignment = TextHorizontalAlignment::LEFT
				}
			},
			WidgetBlueprint{
				.x = 5, .y = 10, .w = 50, .h = 5,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = question2,
					.horizontal_alignment = TextHorizontalAlignment::LEFT
				}
			},
			WidgetBlueprint{
				.x = 5, .y = 30, .w = 15, .h = 5,
				.variant = TextBlueprint{
					.text = decline_text,
					.on_action = [](MAYBE const Text& self) -> Action {
						return make_return_action<bool>(false);
					}
				}
			},
			WidgetBlueprint{
				.x = 25, .y = 30, .w = 30, .h = 5,
				.variant = TextBlueprint{
					.text = accept_text,
					.on_action = [](MAYBE const Text& self) -> Action {
						return make_return_action<bool>(true);
					}
				}
			}
		}
	};

	bool selection;
	State::create_execute_sync(&blueprint, M2_GAME.dimensions().game_and_hud.ratio({0.15f, 0.15f, 0.7f, 0.7f}))
		.if_return<bool>([&](auto result) { selection = result; });
	return selection;
}

bool ask_for_confirmation_bottom(const std::string& question, const std::string& accept_text, const std::string& decline_text, m2::sdl::TextureUniquePtr background_texture) {
	auto blueprint = Blueprint{
		.w = 44, .h = 12,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			WidgetBlueprint{
				.x = 1, .y = 1, .w = 32, .h = 10,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = question,
					.horizontal_alignment = TextHorizontalAlignment::LEFT
				}
			},
			WidgetBlueprint{
				.x = 34, .y = 1, .w = 4, .h = 10,
				.variant = TextBlueprint{
					.text = decline_text,
					.on_action = [](MAYBE const Text& self) -> Action {
						return make_return_action<bool>(false);
					}
				}
			},
			WidgetBlueprint{
				.x = 39, .y = 1, .w = 4, .h = 10,
				.variant = TextBlueprint{
					.text = accept_text,
					.on_action = [](MAYBE const Text& self) -> Action {
						return make_return_action<bool>(true);
					}
				}
			}
		}
	}; // 1 + 32 + 1 + 4 + 1 + 4 + 1

	bool selection;
	State::create_execute_sync(&blueprint, M2_GAME.dimensions().game.ratio({0.0f, 0.9f, 1.0f, 0.1f}), std::move(background_texture))
		.if_return<bool>([&](auto result) { selection = result; });
	return selection;
}

void display_blocking_message(const std::string& message) {
	auto blueprint = Blueprint{
		.w = 60, .h = 40,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			WidgetBlueprint{
				.x = 5, .y = 5, .w = 50, .h = 20,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = message,
					.horizontal_alignment = TextHorizontalAlignment::LEFT,
					.vertical_alignment = TextVerticalAlignment::TOP,
					.wrapped_font_size_in_units = 3.0f
				}
			},
			WidgetBlueprint{
				.x = 5, .y = 30, .w = 50, .h = 5,
				.variant = TextBlueprint{
					.text = "OK",
					.on_action = [](MAYBE const Text& self) -> Action {
						return make_return_action();
					}
				}
			}
		}
	};

	State::create_execute_sync(&blueprint, M2_GAME.dimensions().game_and_hud.ratio({0.15f, 0.15f, 0.7f, 0.7f}));
}

m2::RGB generate_player_color(unsigned index) {
	switch (index) {
		case 0:
			return m2::RGB{0, 255, 255};
		case 1:
			return m2::RGB{255, 0, 255};
		case 2:
			return m2::RGB{255, 255, 0};
		case 3:
			return m2::RGB{0, 255, 0};
		default:
			throw M2_ERROR("Invalid player index");
	}
}

std::string generate_player_name(unsigned index) {
	switch (index) {
		case 0:
			return "Cyan";
		case 1:
			return "Pink";
		case 2:
			return "Yellow";
		case 3:
			return "Green";
		default:
			throw M2_ERROR("Invalid player index");
	}
}
