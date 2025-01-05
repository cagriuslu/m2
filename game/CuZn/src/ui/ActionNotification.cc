#include <cuzn/ui/ActionNotification.h>
#include <m2/ui/UiPanel.h>
#include <m2/ui/widget/TextBlueprint.h>
#include <m2/ui/widget/Text.h>
#include <cuzn/ui/Detail.h>
#include <m2/Log.h>
#include <m2/Game.h>

using namespace m2;

void display_action_notification(const m2g::pb::ServerCommand::ActionNotification& action_notification) {
	LOG_INFO("Displaying action notification", action_notification.player_index(), action_notification.notification());

	auto blueprint = UiPanelBlueprint{
		.name = "ActionNotification",
		.w = 60, .h = 60,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			UiWidgetBlueprint{
				.x = 5, .y = 5, .w = 50, .h = 5,
				.border_width = 0,
				.variant = widget::TextBlueprint{
					.text = generate_player_name(action_notification.player_index()),
					.wrapped_font_size_in_units = 3.0f,
					.color = generate_player_color(action_notification.player_index())
				}
			},
			UiWidgetBlueprint{
				.x = 5, .y = 15, .w = 50, .h = 30,
				.border_width = 0,
				.variant = widget::TextBlueprint{
					.text = action_notification.notification(),
					.horizontal_alignment = TextHorizontalAlignment::LEFT,
					.vertical_alignment = TextVerticalAlignment::TOP,
					.wrapped_font_size_in_units = 3.0f
				}
			},
			UiWidgetBlueprint{
				.x = 5, .y = 50, .w = 50, .h = 5,
				.variant = widget::TextBlueprint{
					.text = "Close",
					.wrapped_font_size_in_units = 3.0f,
					.on_action = [](MAYBE const widget::Text& self) -> UiAction {
						return MakeReturnAction();
					}
				}
			}
		}
	};

	const auto area = RectF{
			M2_GAME.Dimensions().HudWidthToGameAndHudWidthRatio() + 0.016875f,
			0.6f - 0.03f,
			0.3f,
			0.4f};

	// Play sound
	M2_GAME.audio_manager->play(&M2_GAME.songs[m2g::pb::SONG_NOTIFICATION_SOUND], m2::AudioManager::ONCE);

	// Remove panel if already created (or created and killed)
	if (M2G_PROXY.actionNotificationPanel) {
		M2_LEVEL.RemoveCustomNonblockingUiPanel(*M2G_PROXY.actionNotificationPanel);
		M2G_PROXY.actionNotificationPanel.reset();
	}
	M2G_PROXY.actionNotificationPanel = M2_LEVEL.AddCustomNonblockingUiPanel(
			std::make_unique<UiPanelBlueprint>(blueprint), area);
}
