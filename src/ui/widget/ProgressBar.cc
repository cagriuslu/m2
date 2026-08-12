#include <m2/ui/widget/ProgressBar.h>
#include <m2/thirdparty/video/Shapes.h>
#include <m2/Game.h>

using namespace m2;
using namespace m2::widget;

ProgressBar::ProgressBar(UiPanel* parent, const UiWidgetBlueprint* blueprint) : UiWidget(parent, blueprint), _progress(std::clamp(std::get<ProgressBarBlueprint>(blueprint->variant).initial_progress, 0.0f, 1.0f)), _markerVisual(std::get<ProgressBarBlueprint>(blueprint->variant).markerVisual) {
	if (VariantBlueprint().onCreate) {
		VariantBlueprint().onCreate(*this);
	}
}

RectF ProgressBar::MarkerVisualRect() const {
	const auto drawableArea = drawable_area();
	const auto markerVisualLeftEdge = drawableArea.x + (drawableArea.w - drawableArea.h) * _progress;
	return RectF{markerVisualLeftEdge, drawableArea.y, drawableArea.h, drawableArea.h};
}

void ProgressBar::SetMarkerVisual(ProgressBarMarkerVisual markerVisual) {
	_markerVisual = std::move(markerVisual);
}

UiAction ProgressBar::OnUpdate() {
	auto& pb_blueprint = std::get<ProgressBarBlueprint>(blueprint->variant);
	if (pb_blueprint.onUpdate) {
		pb_blueprint.onUpdate(*this);
	}
	return MakeContinueAction();
}

void ProgressBar::OnDraw() {
	auto& pb_blueprint = std::get<ProgressBarBlueprint>(blueprint->variant);
	// Background
	draw_background_color();
	// Bar
	const auto filled_dstrect = m2::RectF{Rect().x, Rect().y, Rect().w * _progress, Rect().h};
	m2::thirdparty::video::FillRectangle(M2_GAME.GetRenderer(), filled_dstrect, pb_blueprint.bar_color);
	// Marker visual
	std::visit(overloaded {
		[](MAYBE const std::monostate& monostate) {},
		[this](const m2g::pb::SpriteType spriteType) {
			if (spriteType) {
				DrawSpriteOrTextLabel(M2_GAME.GetSpriteOrTextLabel(spriteType), MarkerVisualRect());
			}
		},
		[this](const std::function<void(const ProgressBar&)>& onDraw) {
			if (onDraw) { onDraw(*this); }
		}
	}, _markerVisual);
	// Foreground
	draw_border(Rect(), vertical_border_width_px(), horizontal_border_width_px());
}
