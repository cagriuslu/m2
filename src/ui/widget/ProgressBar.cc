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
	return make_continue_action();
}

void ProgressBar::on_draw() {
	auto& pb_blueprint = std::get<ProgressBarBlueprint>(blueprint->variant);
	// Background
	draw_background_color();
	// Bar
	auto filled_dstrect = SDL_Rect{
			rect_px.x,
			rect_px.y,
			(int)roundf((float)rect_px.w * _progress),
			rect_px.h
	};
	SDL_SetRenderDrawColor(M2_GAME.renderer, pb_blueprint.bar_color.r, pb_blueprint.bar_color.g, pb_blueprint.bar_color.b, pb_blueprint.bar_color.a);
	SDL_RenderFillRect(M2_GAME.renderer, &filled_dstrect);
	// Foreground
	draw_border(rect_px, vertical_border_width_px(), horizontal_border_width_px());
}
