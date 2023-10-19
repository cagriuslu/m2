#include <m2/ui/widget/ProgressBar.h>
#include <m2/Game.h>

using namespace m2::ui;
using namespace m2::ui::widget;

ProgressBar::ProgressBar(State* parent, const WidgetBlueprint* blueprint) : Widget(parent, blueprint), _progress(std::get<ProgressBarBlueprint>(blueprint->variant).initial_progress) {}

Action ProgressBar::on_update() {
	auto& pb_blueprint = std::get<ProgressBarBlueprint>(blueprint->variant);
	if (pb_blueprint.on_update) {
		_progress = pb_blueprint.on_update(*this);
	}
	return Action::CONTINUE;
}

void ProgressBar::on_draw() {
	auto& pb_blueprint = std::get<ProgressBarBlueprint>(blueprint->variant);
	// Background
	draw_background_color(rect_px, blueprint->background_color);
	// Bar
	auto filled_dstrect = SDL_Rect{
			rect_px.x,
			rect_px.y,
			(int)roundf((float)rect_px.w * _progress),
			rect_px.h
	};
	SDL_SetRenderDrawColor(GAME.renderer, pb_blueprint.bar_color.r, pb_blueprint.bar_color.g, pb_blueprint.bar_color.b, pb_blueprint.bar_color.a);
	SDL_RenderFillRect(GAME.renderer, &filled_dstrect);
	// Foreground
	draw_border(rect_px, blueprint->border_width_px);
}
