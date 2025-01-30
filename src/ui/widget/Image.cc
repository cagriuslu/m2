#include <m2/ui/widget/Image.h>
#include <m2/Game.h>

using namespace m2;
using namespace m2::widget;

Image::Image(UiPanel* parent, const UiWidgetBlueprint* blueprint) : AbstractButton(parent, blueprint),
		_spriteType(VariantBlueprint().initial_sprite) {
	if (VariantBlueprint().onCreate) {
		VariantBlueprint().onCreate(*this);
	}
}

UiAction Image::UpdateContent() {
	auto& image_blueprint = std::get<ImageBlueprint>(blueprint->variant);
	if (image_blueprint.onUpdate) {
		auto[action, opt_sprite] = image_blueprint.onUpdate(*this);
		if (action.IsContinue() && opt_sprite) {
			_spriteType = *opt_sprite;
		}
		return std::move(action);
	} else {
		return MakeContinueAction();
	}
}

void Image::Draw() {
	draw_background_color();
	if (_spriteType) {
		DrawSpriteOrTextLabel(M2_GAME.GetSpriteOrTextLabel(_spriteType), Rect());
	}
	draw_border(Rect(), vertical_border_width_px(), horizontal_border_width_px(), depressed ? SDL_Color{127, 127, 127, 255} : SDL_Color{255, 255, 255, 255});
}

void Image::SetSpriteType(const m2g::pb::SpriteType s) {
	_spriteType = s;
}
