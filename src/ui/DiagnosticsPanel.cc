#include <m2/ui/DiagnosticsPanel.h>
#include <m2/Game.h>
#include <m2/ui/widget/Text.h>
#include <m2/common/Chrono.h>
#include <chrono>
#include <format>

namespace {
	constexpr auto DIAGNOSTICS_SAMPLE_PERIOD = std::chrono::seconds{2};

	struct DiagnosticsPanelState : m2::UiPanelStateBase {
		m2::Stopwatch sampleStopwatch; // Wall-clock; starts on construction
		unsigned lastPhySimulationCount{};
		unsigned lastGfxUpdateCount{};
		m2::widget::Text* phyText{};
		m2::widget::Text* gfxText{};
	};
}

const m2::UiPanelBlueprint m2::DIAGNOSTICS_PANEL_BLUEPRINT = {
	.name = "Diagnostics",
	.w = 1, .h = 2,
	.border_width = 0,
	.ignore_events = true,
	.onCreate = [](UiPanel& panel) {
		auto state = std::make_unique<DiagnosticsPanelState>();
		state->lastPhySimulationCount = M2_GAME.GetPhySimulationCount();
		state->lastGfxUpdateCount = M2_GAME.GetGfxUpdateCount();
		state->phyText = panel.FindWidget<widget::Text>("PHY");
		state->gfxText = panel.FindWidget<widget::Text>("GFX");
		panel.state = std::move(state);
	},
	.onUpdate = [](UiPanel& panel) -> UiAction {
		auto& state = dynamic_cast<DiagnosticsPanelState&>(*panel.state);
		if (state.sampleStopwatch.HasTimePassed(DIAGNOSTICS_SAMPLE_PERIOD)) {
			const auto elapsedSeconds = ToDurationF(state.sampleStopwatch.Reset());
			const auto phyCountNow = M2_GAME.GetPhySimulationCount();
			const auto gfxCountNow = M2_GAME.GetGfxUpdateCount();
			const auto phyRatePerSecond = static_cast<float>(phyCountNow - state.lastPhySimulationCount) / elapsedSeconds;
			const auto gfxRatePerSecond = static_cast<float>(gfxCountNow - state.lastGfxUpdateCount) / elapsedSeconds;
			if (state.phyText) {
				state.phyText->set_text(std::format("PHY:{:.1f}", phyRatePerSecond));
			}
			if (state.gfxText) {
				state.gfxText->set_text(std::format("GFX:{:.1f}", gfxRatePerSecond));
			}
			state.lastPhySimulationCount = phyCountNow;
			state.lastGfxUpdateCount = gfxCountNow;
		}
		return MakeContinueAction();
	},
	.widgets = {
		UiWidgetBlueprint{
			.name = "PHY",
			.x = 0, .y = 0, .w = 1, .h = 1,
			.border_width = 0,
			.variant = widget::TextBlueprint{
				.text = "PHY:0.0",
				.horizontal_alignment = TextHorizontalAlignment::LEFT,
				.wrapped_font_size_in_units = 0.8f,
				.color = RGB{128, 128, 128}
			}
		},
		UiWidgetBlueprint{
			.name = "GFX",
			.x = 0, .y = 1, .w = 1, .h = 1,
			.border_width = 0,
			.variant = widget::TextBlueprint{
				.text = "GFX:0.0",
				.horizontal_alignment = TextHorizontalAlignment::LEFT,
				.wrapped_font_size_in_units = 0.8f,
				.color = RGB{128, 128, 128}
			}
		}
	}
};

const m2::RectF m2::DIAGNOSTICS_PANEL_AREA = {0.0f, 0.0f, 0.08f, 0.045f};
