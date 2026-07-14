#include <m2/ui/widget/CheckboxWithImage.h>
#include <m2/Game.h>

using namespace m2;
using namespace m2::widget;

CheckboxWithImage::CheckboxWithImage(UiPanel* parent, const UiWidgetBlueprint* blueprint)
    : AbstractButton(parent, blueprint),
      _state(std::get<CheckboxWithImageBlueprint>(blueprint->variant).initial_state),
      _content(std::get<CheckboxWithImageBlueprint>(blueprint->variant).content) {
	if (VariantBlueprint().onCreate) {
		VariantBlueprint().onCreate(*this);
	}
}

void CheckboxWithImage::SetSpriteType(const m2g::pb::SpriteType s) { _content = s; }

UiAction CheckboxWithImage::OnUpdate() {
	if (VariantBlueprint().onUpdate) {
		auto [action, opt_sprite] = VariantBlueprint().onUpdate(*this);
		if (action.IsContinue() && opt_sprite) {
			_content = *opt_sprite;
		}
		return std::move(action);
	} else {
		return MakeContinueAction();
	}
}

void CheckboxWithImage::OnDraw() {
	draw_background_color();
	const auto accentColor = _state ? RGBA{127, 127, 127, 255} : RGBA{255, 255, 255, 255};
	std::visit(overloaded {
		[this](const m2g::pb::SpriteType spriteType) {
			if (spriteType) { DrawSpriteOrTextLabel(M2_GAME.GetSpriteOrTextLabel(spriteType), Rect()); }
		},
		[this](const std::function<void(const CheckboxWithImage&)>& onDraw) {
			if (onDraw) { onDraw(*this); }
		}
	}, _content);
	draw_border(Rect(), vertical_border_width_px(), horizontal_border_width_px(), accentColor);
}
