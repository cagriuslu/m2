#include <cuzn/ui/IndustrySelection.h>
#include <m2/ui/UiPanelBlueprint.h>
#include <m2/Game.h>
#include <m2/Log.h>

using namespace m2;
using namespace m2::widget;

std::optional<m2g::pb::ItemType> ask_for_industry_selection(m2g::pb::ItemType industry_1, m2g::pb::ItemType industry_2) {
	LOG_INFO("Asking player to select an industry...");

	auto blueprint = UiPanelBlueprint{
		.name = "IndustrySelection",
		.w = 60, .h = 40,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			UiWidgetBlueprint{
				.x = 57, .y = 0, .w = 3, .h = 3,
				.variant = TextBlueprint{
					.text = "X",
					.onAction = [](MAYBE const Text& self) -> UiAction {
						return MakeReturnAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 5, .y = 14, .w = 50, .h = 5,
				.variant = TextBlueprint{
					.text = M2_GAME.GetNamedItem(industry_1).in_game_name(),
					.onAction = [industry_1](MAYBE const Text& self) -> UiAction {
						return MakeReturnAction<m2g::pb::ItemType>(industry_1);
					}
				}
			},
			UiWidgetBlueprint{
				.x = 5, .y = 21, .w = 50, .h = 5,
				.variant = TextBlueprint{
					.text = M2_GAME.GetNamedItem(industry_2).in_game_name(),
					.onAction = [industry_2](MAYBE const Text& self) -> UiAction {
						return MakeReturnAction<m2g::pb::ItemType>(industry_2);
					}
				}
			}
		}
	};

	std::optional<m2g::pb::ItemType> selected_industry;
	auto background = M2_GAME.DrawGameToTexture(M2_LEVEL.GetCamera()->position);
	UiPanel::create_and_run_blocking(std::make_unique<m2::UiPanelBlueprint>(blueprint), RectF{0.15f, 0.15f, 0.7f, 0.7f}, std::move(background))
			.IfQuit([] { M2_GAME.quit = true; })
			.IfVoidReturn([&]() {
				LOG_INFO("Industry selection cancelled");
			})
			.IfReturn<m2g::pb::ItemType>([&](auto industry) {
				LOG_INFO("Industry selected", m2g::pb::ItemType_Name(industry));
				selected_industry = industry;
			});
	return selected_industry;
}
