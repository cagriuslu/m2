#include <m2/ui/widget/Image.h>
#include <m2/Game.h>

using namespace m2;
using namespace m2::widget;

Image::Image(UiPanel* parent, const UiWidgetBlueprint* blueprint) : AbstractButton(parent, blueprint), _content(VariantBlueprint().content) {
	if (VariantBlueprint().onCreate) {
		VariantBlueprint().onCreate(*this);
	}
}

UiAction Image::OnUpdate() {
	auto& image_blueprint = std::get<ImageBlueprint>(blueprint->variant);
	if (image_blueprint.onUpdate) {
		auto[action, opt_sprite] = image_blueprint.onUpdate(*this);
		if (action.IsContinue() && opt_sprite) {
			_content = *opt_sprite;
		}
		return std::move(action);
	} else {
		return MakeContinueAction();
	}
}

void Image::OnDraw() {
	draw_background_color();
	std::visit(overloaded {
		[this](const m2g::pb::SpriteType spriteType) {
			if (spriteType) {
				DrawSpriteOrTextLabel(M2_GAME.GetSpriteOrTextLabel(spriteType), Rect());
			}
		},
		[this](const std::function<void(const Image&)>& onDraw) {
			if (onDraw) { onDraw(*this); }
		}
	}, _content);
	draw_border(Rect(), vertical_border_width_px(), horizontal_border_width_px(), depressed ? SDL_Color{127, 127, 127, 255} : SDL_Color{255, 255, 255, 255});
}

void Image::SetSpriteType(const m2g::pb::SpriteType s) {
	_content = s;
}
