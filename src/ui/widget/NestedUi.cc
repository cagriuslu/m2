#include <m2/ui/widget/NestedUi.h>
#include <m2/ui/State.h>
#include <m2/ui/Blueprint.h>
#include <m2/Game.h>
#include <m2/Log.h>

using namespace m2::ui;
using namespace m2::ui::widget;

NestedUi::NestedUi(State* parent, const WidgetBlueprint *blueprint) : Widget(parent, blueprint) {
	const auto& nested_blueprint = std::get<NestedUiBlueprint>(blueprint->variant);

	if (nested_blueprint.inner_w == 0 || blueprint->w < nested_blueprint.inner_w) {
		throw M2ERROR("Unexpected inner width");
	}
	if (nested_blueprint.inner_h == 0 || blueprint->h < nested_blueprint.inner_h) {
		throw M2ERROR("Unexpected inner height");
	}

	_ui = std::make_unique<State>(nested_blueprint.ui);
}

void NestedUi::on_position_update(const SDL_Rect &rect_px_) {
	rect_px = rect_px_;

	const auto& nested_blueprint = std::get<NestedUiBlueprint>(blueprint->variant);
	if (nested_blueprint.inner_w == 0 && nested_blueprint.inner_h == 0) {
		_ui->update_positions(rect_px_);
	} else {
		// Update position based on inner_w and inner_h
		auto fake_rect = SDL_Rect{
				.x = rect_px.x - _inner_x * rect_px.w / nested_blueprint.inner_w,
				.y = rect_px.y - _inner_y * rect_px.h / nested_blueprint.inner_h,
				.w = rect_px.w * nested_blueprint.ui->w / nested_blueprint.inner_w,
				.h = rect_px.h * nested_blueprint.ui->h / nested_blueprint.inner_h
		};
		_ui->update_positions(calculate_widget_rect(fake_rect, nested_blueprint.ui->w, nested_blueprint.ui->h, 0, 0, nested_blueprint.ui->w, nested_blueprint.ui->h));
	}
}

Action NestedUi::on_event(Events &events) {
	auto nested_action = _ui->handle_events(events);
	if (nested_action == Action::CONTINUE) {
		// Handle scroll, value is negative on scroll down
		if (auto vertical_scroll_count = -events.pop_mouse_wheel_vertical_scroll(RectI{rect_px}); vertical_scroll_count) {
			_inner_y = std::clamp(_inner_y + vertical_scroll_count, 0, _ui->blueprint->h - std::get<NestedUiBlueprint>(blueprint->variant).inner_h);
			on_position_update(rect_px);
		}
		if (auto horizontal_scroll_count = events.pop_mouse_wheel_horizontal_scroll(RectI{rect_px}); horizontal_scroll_count) {
			_inner_x = std::clamp(_inner_x + horizontal_scroll_count, 0, _ui->blueprint->w - std::get<NestedUiBlueprint>(blueprint->variant).inner_w);
			on_position_update(rect_px);
		}
	}
	return nested_action;
}

Action NestedUi::on_update() {
	return _ui->update_contents();
}

void NestedUi::on_draw() {
	SDL_RenderSetClipRect(GAME.renderer, &rect_px);
	_ui->draw();
	SDL_RenderSetClipRect(GAME.renderer, nullptr);
}
