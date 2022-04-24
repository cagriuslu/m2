#include <m2/ui/widget/ProgressBar.h>
#include <m2/ui/WidgetBlueprint.h>
#include <m2/ui/UI.h>
#include <m2/Game.hh>

m2::ui::wdg::ProgressBarState::ProgressBarState(const WidgetBlueprint* blueprint) : WidgetState(blueprint),
	progress(std::get<ProgressBarBlueprint>(blueprint->variant).initial_progress) {}

m2::ui::Action m2::ui::wdg::ProgressBarState::update_content() {
	auto& pb_blueprint = std::get<ProgressBarBlueprint>(blueprint->variant);
	if (pb_blueprint.update_callback) {
		progress = pb_blueprint.update_callback();
	}
	return Action::CONTINUE;
}

void m2::ui::wdg::ProgressBarState::draw() {
	auto& pb_blueprint = std::get<ProgressBarBlueprint>(blueprint->variant);
	// Background
	UIState::draw_background_color(rect_px, blueprint->background_color);
	// Bar
	auto filled_dstrect = SDL_Rect{
		rect_px.x,
		rect_px.y,
		(int)roundf((float)rect_px.w * progress),
		rect_px.h
	};
	SDL_SetRenderDrawColor(GAME.sdlRenderer, pb_blueprint.bar_color.r, pb_blueprint.bar_color.g,
		pb_blueprint.bar_color.b, pb_blueprint.bar_color.a);
	SDL_RenderFillRect(GAME.sdlRenderer, &filled_dstrect);
	// Foreground
	UIState::draw_border(rect_px, blueprint->border_width_px);
}
