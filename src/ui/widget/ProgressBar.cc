#include <m2/ui/widget/ProgressBar.h>
#include <m2/Game.h>

using namespace m2;
using namespace m2::widget;

ProgressBar::ProgressBar(UiPanel* parent, const UiWidgetBlueprint* blueprint) : UiWidget(parent, blueprint), _progress(std::get<ProgressBarBlueprint>(blueprint->variant).initial_progress) {
	if (VariantBlueprint().onCreate) {
		VariantBlueprint().onCreate(*this);
	}
}

UiAction ProgressBar::UpdateContent() {
	auto& pb_blueprint = std::get<ProgressBarBlueprint>(blueprint->variant);
	if (pb_blueprint.onUpdate) {
		pb_blueprint.onUpdate(*this);
	}
	return MakeContinueAction();
}

void ProgressBar::Draw() {
	auto& pb_blueprint = std::get<ProgressBarBlueprint>(blueprint->variant);
	// Background
	draw_background_color();
	// Bar
	auto filled_dstrect = SDL_Rect{
			Rect().x,
			Rect().y,
			(int)roundf((float)Rect().w * _progress),
			Rect().h
	};
	SDL_SetRenderDrawColor(M2_GAME.renderer, pb_blueprint.bar_color.r, pb_blueprint.bar_color.g, pb_blueprint.bar_color.b, pb_blueprint.bar_color.a);
	SDL_RenderFillRect(M2_GAME.renderer, &filled_dstrect);
	// Foreground
	draw_border(Rect(), vertical_border_width_px(), horizontal_border_width_px());
}
