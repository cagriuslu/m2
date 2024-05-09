#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>
#include <m2/ui/widget/TextListSelection.h>
#include <m2g/Proxy.h>
#include <cuzn/Ui.h>

using namespace m2::ui;
using namespace m2::ui::widget;
using namespace m2g;
using namespace m2g::pb;

namespace {
	auto quit_button_action = [](MAYBE const widget::Text& self) { return make_quit_action(); };
}  // namespace

static TextBlueprint client_status = {
	.initial_text = "CONNECTING",
	.on_update = [](MAYBE const Text& self) -> std::pair<Action, std::optional<std::string>> {
		if (M2_GAME.client_thread().is_connected()) {
			return std::make_pair(make_continue_action(), "CONNECTED");
		} else if (M2_GAME.client_thread().is_ready()) {
			return std::make_pair(make_continue_action(), "READY");
		} else if (M2_GAME.client_thread().is_started()) {
			auto server_update = M2_GAME.client_thread().peek_unprocessed_server_update();
			m2_succeed_or_throw_message(server_update, "Client state is Started, but ServerUpdate not found");
			auto player_count = server_update->player_object_ids_size();

			const auto expect_success =
				M2_GAME.load_multi_player_as_guest(M2_GAME.levels_dir / "Map.json", std::to_string(player_count));
			m2_succeed_or_throw_error(expect_success);

			return std::make_pair(make_return_action(), std::nullopt);
		} else {
			return std::make_pair(make_continue_action(), "CONNECTING...");
		}
	}
};
static TextBlueprint ready_button = {
	.initial_text = "...",
	.on_update = [](MAYBE const Text& self) -> std::pair<Action, std::optional<std::string>> {
		if (M2_GAME.client_thread().is_connected()) {
			return std::make_pair(make_continue_action(), "SET READY");
		} else if (M2_GAME.client_thread().is_ready()) {
			return std::make_pair(make_continue_action(), "CLEAR READY");
		} else {
			return std::make_pair(make_continue_action(), "...");
		}
	},
	.on_action = [](MAYBE const Text& self) -> Action {
		if (M2_GAME.client_thread().is_connected()) {
			M2_GAME.client_thread().set_ready_blocking(true);
		} else if (M2_GAME.client_thread().is_ready()) {
			M2_GAME.client_thread().set_ready_blocking(false);
		}
		return make_continue_action();
	}
};
static const Blueprint client_lobby = {
	.w = 160,
	.h = 90,
	.border_width_px = 0,
	.background_color = {20, 20, 20, 255},
	.widgets = {
		WidgetBlueprint{.x = 60, .y = 30, .w = 40, .h = 10, .border_width_px = 0, .variant = client_status},
		WidgetBlueprint{.x = 60, .y = 50, .w = 40, .h = 10, .border_width_px = 1, .variant = ready_button}
	}
};
static const Blueprint ip_port_form = {
	.w = 160,
	.h = 90,
	.border_width_px = 0,
	.background_color = {20, 20, 20, 255},
	.widgets = {
		WidgetBlueprint{.x = 55, .y = 30, .w = 10, .h = 10,
			.border_width_px = 0,
			.variant = TextBlueprint{.initial_text = "IP"}
		},
		WidgetBlueprint{
			.initially_focused = true,
			.x = 65,
			.y = 30,
			.w = 40,
			.h = 10,
			.border_width_px = 1,
			.padding_width_px = 5,
			.variant = TextInputBlueprint{.initial_text = "127.0.0.1"}
		},
		WidgetBlueprint{
			.x = 70,
			.y = 50,
			.w = 20,
			.h = 10,
			.border_width_px = 1,
			.padding_width_px = 5,
			.variant = TextBlueprint{
				.initial_text = "CONNECT", .on_action = [](MAYBE const widget::Text& self) {
					auto* ip_input_widget = self.parent().find_first_widget_of_type<TextInput>();
					M2_GAME.join_game(m2::mplayer::Type::TurnBased, ip_input_widget->text_input());
					return m2::ui::State::create_execute_sync(&client_lobby);
				}
			}
		}
	}
};

static TextBlueprint client_count = {
	.initial_text = "0",
	.on_update = [](MAYBE const Text& self) -> std::pair<Action, std::optional<std::string>> {
		auto client_count = M2_GAME.server_thread().client_count();
		if (client_count < 2) {
			return std::make_pair(make_continue_action(), std::to_string(client_count));
		} else {
			return std::make_pair(make_continue_action(), std::to_string(client_count) + " - START!");
		}
	},
	.on_action = [](MAYBE const Text& self) -> Action {
		if (2 <= M2_GAME.server_thread().client_count()) {
			LOG_INFO("Enough clients have connected");
			if (M2_GAME.server_thread().close_lobby()) {
				auto client_count = M2_GAME.server_thread().client_count();
				const auto expect_success =
					M2_GAME.load_multi_player_as_host(M2_GAME.levels_dir / "Map.json", std::to_string(client_count));
				m2_succeed_or_throw_error(expect_success);
				return make_return_action();  // TODO Return value
			}
		}
		return make_continue_action();
	}
};
static const Blueprint server_lobby = {
	.w = 160,
	.h = 90,
	.border_width_px = 0,
	.background_color = {20, 20, 20, 255},
	.widgets = {
		WidgetBlueprint{
			.x = 40,
			.y = 40,
			.w = 40,
			.h = 10,
			.border_width_px = 0,
			.padding_width_px = 5,
			.variant = TextBlueprint{.initial_text = "Client count:"}
		},
		WidgetBlueprint{
			.x = 80, .y = 40, .w = 40, .h = 10,
			.border_width_px = 1,
			.padding_width_px = 5,
			.variant = client_count
		},
	}
};

const Blueprint main_menu_blueprint = {
	.w = 160,
	.h = 90,
	.border_width_px = 0,
	.background_color = {20, 20, 20, 255},
	.widgets = {
		WidgetBlueprint{
			.x = 70, .y = 20, .w = 20, .h = 10,
			.border_width_px = 1,
			.padding_width_px = 5,
			.variant = TextBlueprint{
				.initial_text = "JOIN", .on_action = [](MAYBE const widget::Text& self) {
					return m2::ui::State::create_execute_sync(&ip_port_form);
				}
			}
		},
		WidgetBlueprint{
			.x = 70, .y = 40, .w = 20, .h = 10,
			.border_width_px = 1,
			.padding_width_px = 5,
			.variant = TextBlueprint{
				.initial_text = "HOST", .on_action = [](MAYBE const widget::Text& self) {
					M2_GAME.host_game(m2::mplayer::Type::TurnBased, 4);
					return m2::ui::State::create_execute_sync(&server_lobby);
				}
			}
		},
		WidgetBlueprint{
			.x = 70, .y = 60, .w = 20, .h = 10,
			.border_width_px = 1,
			.padding_width_px = 5,
			.variant = TextBlueprint{
				.initial_text = "QUIT", .kb_shortcut = SDL_SCANCODE_Q, .on_action = quit_button_action
			}
		}
	}
};

const Blueprint pause_menu_blueprint = {
	.w = 100,
	.h = 100,
	.background_color = {.r = 20, .g = 20, .b = 20, .a = 255},
	.widgets = {
		WidgetBlueprint{
			.x = 45, .y = 35, .w = 10, .h = 10,
			.border_width_px = 1,
			.padding_width_px = 5,
			.variant = TextBlueprint{
				.initial_text = "RESUME", .on_action = [](MAYBE const widget::Text& self) {
					return make_return_action();  // TODO Return value
				}
			}
		},
		WidgetBlueprint{
			.x = 45,
			.y = 55,
			.w = 10,
			.h = 10,
			.border_width_px = 1,
			.padding_width_px = 5,
			.variant = TextBlueprint{
				.initial_text = "QUIT",
				.kb_shortcut = SDL_SCANCODE_Q,
				.on_action = quit_button_action,
			}
		}
	}
};

const Blueprint left_hud_blueprint = {
	.w = 19,
	.h = 72,
	.border_width_px = 0,
	.background_color = {0, 0, 0, 255},
	.widgets = {
		WidgetBlueprint{
			.x = 2, .y = 2, .w = 15, .h = 6,
			.variant = TextBlueprint{
				.initial_text = "Build",
				.font_size = 4.5f,
				.on_action = [](MAYBE const Text& self) -> Action {
					if (M2_GAME.client_thread().is_our_turn()) {
						LOG_INFO("Beginning BuildJourney");
						m2g::Proxy::get_instance().user_journey.emplace(cuzn::BuildJourney{});
					} else {
						M2_LEVEL.display_message("It's not your turn");
					}
					return make_continue_action();
				}
			}
		},
		WidgetBlueprint{
			.x = 2, .y = 9, .w = 15, .h = 6,
			.variant = TextBlueprint{
				.initial_text = "Network",
				.font_size = 4.5f,
				.on_action = [](MAYBE const Text& self) -> Action {
					if (M2_GAME.client_thread().is_our_turn()) {
						LOG_INFO("Beginning BuildJourney");
						m2g::Proxy::get_instance().user_journey.emplace(cuzn::NetworkJourney{});
					} else {
						M2_LEVEL.display_message("It's not your turn");
					}
					return make_continue_action();
				}
			}
		},
		WidgetBlueprint{
			.x = 2, .y = 16, .w = 15, .h = 6, .variant = TextBlueprint{.initial_text = "Develop", .font_size = 4.5f}
		},
		WidgetBlueprint{
			.x = 2, .y = 23, .w = 15, .h = 6, .variant = TextBlueprint{.initial_text = "Sell", .font_size = 4.5f}
		},
		WidgetBlueprint{
			.x = 2,
			.y = 30,
			.w = 15,
			.h = 6,
			.variant =
			TextBlueprint{
				.initial_text = "Loan",
				.font_size = 4.5f,
				.on_action = [](MAYBE const m2::ui::widget::Text& self) -> m2::ui::Action {
					if (M2_GAME.client_thread().is_our_turn()) {
						pb::ClientCommand cc;
						cc.mutable_loan_action();
						M2_GAME.client_thread().queue_client_command(cc);
					}
					return make_continue_action();
				}
			}
		},
		WidgetBlueprint{
			.x = 2, .y = 37, .w = 15, .h = 6, .variant = TextBlueprint{.initial_text = "Scout", .font_size = 4.5f}
		}
	}
};

const Blueprint tiles_blueprint = {
	.w = 60,
	.h = 40,
	.border_width_px = 1,
	.background_color = {0, 0, 0, 255},
	.widgets = {
		WidgetBlueprint{
			.x = 57,
			.y = 0,
			.w = 3,
			.h = 3,
			.variant =
			TextBlueprint{
				.initial_text = "X",
				.on_action = [](MAYBE const m2::ui::widget::Text& self) -> m2::ui::Action {
					return make_return_action();
				}
			}
		},
		WidgetBlueprint{
			.name = "IndustryTypeSelection",
			.x = 3,
			.y = 3,
			.w = 18,
			.h = 34,
			.variant =
			TextListSelectionBlueprint{
				.initial_list = {m2g::pb::ItemCategory_Name(m2g::pb::ITEM_CATEGORY_COTTON_MILL_TILE), m2g::pb::ItemCategory_Name(m2g::pb::ITEM_CATEGORY_IRON_WORKS_TILE), m2g::pb::ItemCategory_Name(m2g::pb::ITEM_CATEGORY_BREWERY_TILE), m2g::pb::ItemCategory_Name(m2g::pb::ITEM_CATEGORY_COAL_MINE_TILE), m2g::pb::ItemCategory_Name(m2g::pb::ITEM_CATEGORY_POTTERY_TILE), m2g::pb::ItemCategory_Name(m2g::pb::ITEM_CATEGORY_MANUFACTURED_GOODS_TILE)},
				.line_count = 10,
				.allow_multiple_selection = false,
				.show_scroll_bar = false,
				.on_action = [](const TextListSelection &self) -> Action {
					// Look for the other widget
					auto* tile_selection_widget = self.parent().find_first_widget_by_name<TextListSelection>("TileSelection");
					if (tile_selection_widget) {
						// Trigger the other widget to recreate itself
						tile_selection_widget->recreate();
					}
					return make_continue_action();
				}
			}
		},
		WidgetBlueprint{
			.name = "TileSelection",
			.x = 21,
			.y = 3,
			.w = 18,
			.h = 34,
			.variant =
			TextListSelectionBlueprint{
				.line_count = 10,
				.allow_multiple_selection = false,
				.show_scroll_bar = false,
				.on_create = [](const TextListSelection &self) -> std::optional<TextListSelectionBlueprint::Options> {
					// Look for the other widget
					auto* industry_type_selection_widget = self.parent().find_first_widget_by_name<TextListSelection>("IndustryTypeSelection");
					if (industry_type_selection_widget) {
						// Get selection
						auto selection = industry_type_selection_widget->selection();
						if (selection.size() == 1) {
							ItemCategory cat;
							if (m2g::pb::ItemCategory_Parse(selection[0], &cat)) {
								TextListSelectionBlueprint::Options options;
								for (auto item_it = M2_PLAYER.character().find_items(cat);
									item_it != M2_PLAYER.character().end_items(); ++item_it) {
									options.emplace_back(m2g::pb::ItemType_Name(item_it->type()));
								}
								return options;
							} else {
								throw M2ERROR("Unable to parse ItemCategory");
							}
						}
					}
					// Return empty list
					return TextListSelectionBlueprint::Options{};
				}
			}
		}
	}
};

const Blueprint right_hud_blueprint = {
	.w = 19,
	.h = 72,
	.border_width_px = 0,
	.background_color = {0, 0, 0, 255},
	.widgets = {
		WidgetBlueprint{
			.x = 2,
			.y = 0,
			.w = 15,
			.h = 6,
			.border_width_px = 0,
			.variant =
			TextBlueprint{
				.font_size = 4.5f,
				.alignment = m2::ui::TextAlignment::LEFT,
				.on_update = [](MAYBE const Text& self) -> std::pair<Action, std::optional<std::string>> {
					// Lookup victory points
					auto vp = m2::I(M2_PLAYER.character().get_attribute(VICTORY_POINTS));
					auto text = std::string{"VP:"} + std::to_string(vp);
					return std::make_pair(make_continue_action(), text);
				}
			}
		},
		WidgetBlueprint{
			.x = 2,
			.y = 6,
			.w = 15,
			.h = 6,
			.border_width_px = 0,
			.variant =
			TextBlueprint{
				.font_size = 4.5f,
				.alignment = m2::ui::TextAlignment::LEFT,
				.on_update = [](MAYBE const Text& self) -> std::pair<Action, std::optional<std::string>> {
					// Lookup money
					auto money = m2::I(M2_PLAYER.character().get_resource(MONEY));
					auto text = std::string{"Money:"} + std::to_string(money);
					return std::make_pair(make_continue_action(), text);
				}
			}
		},
		WidgetBlueprint{
			.x = 2,
			.y = 12,
			.w = 15,
			.h = 6,
			.border_width_px = 1,
			.variant =
			TextBlueprint{
				.initial_text = "Cards",
				.font_size = 4.5f,
				.alignment = m2::ui::TextAlignment::LEFT,
				.on_action = [](MAYBE const Text& self) -> Action {
					M2_LEVEL.add_custom_ui_dialog(
						m2::RectF{0.15f, 0.15f, 0.7f, 0.7f},
						std::make_unique<Blueprint>(cuzn::generate_cards_window(false)));
					return make_continue_action();
				}
			}
		},
		WidgetBlueprint{
			.x = 2,
			.y = 18,
			.w = 15,
			.h = 6,
			.border_width_px = 1,
			.variant = TextBlueprint{
				.initial_text = "Tiles",
				.font_size = 4.5f,
				.alignment = m2::ui::TextAlignment::LEFT,
				.on_action = [](MAYBE const Text& self) -> Action {
					M2_LEVEL.add_custom_ui_dialog(m2::RectF{0.05f, 0.05f, 0.9f, 0.9f}, &tiles_blueprint);
					return make_continue_action();
				}
			}
		}
	}
};

const m2::ui::Blueprint* m2g::Proxy::main_menu() { return &main_menu_blueprint; }

const m2::ui::Blueprint* m2g::Proxy::pause_menu() { return &pause_menu_blueprint; }

const m2::ui::Blueprint* m2g::Proxy::left_hud() { return &left_hud_blueprint; }

const m2::ui::Blueprint* m2g::Proxy::right_hud() { return &right_hud_blueprint; }
