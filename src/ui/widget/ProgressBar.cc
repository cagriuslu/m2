#include <m2/ui/widget/ProgressBar.h>
#include <m2/Game.h>

using namespace m2::ui;
using namespace m2::ui::widget;

ProgressBar::ProgressBar(const WidgetBlueprint* blueprint) : Widget(blueprint), progress(std::get<ProgressBarBlueprint>(blueprint->variant).initial_progress) {}

Action ProgressBar::update_content() {
	auto& pb_blueprint = std::get<ProgressBarBlueprint>(blueprint->variant);
	if (pb_blueprint.update_callback) {
		progress = pb_blueprint.update_callback();
	}
	return Action::CONTINUE;
}

void ProgressBar::draw() {
	auto& pb_blueprint = std::get<ProgressBarBlueprint>(blueprint->variant);
	// Background
	draw_background_color(rect_px, blueprint->background_color);
	// Bar
	auto filled_dstrect = SDL_Rect{
			rect_px.x,
			rect_px.y,
			(int)roundf((float)rect_px.w * progress),
			rect_px.h
	};
	SDL_SetRenderDrawColor(GAME.renderer, pb_blueprint.bar_color.r, pb_blueprint.bar_color.g, pb_blueprint.bar_color.b, pb_blueprint.bar_color.a);
	SDL_RenderFillRect(GAME.renderer, &filled_dstrect);
	// Foreground
	draw_border(rect_px, blueprint->border_width_px);
}
