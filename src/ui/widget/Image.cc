#include <m2/ui/widget/Image.h>
#include <m2/Game.h>

using namespace m2;
using namespace m2::ui;
using namespace m2::ui::widget;

Image::Image(Panel* parent, const WidgetBlueprint* blueprint) : AbstractButton(parent, blueprint),
		_spriteType(image_blueprint().initial_sprite) {
	if (image_blueprint().on_create) {
		image_blueprint().on_create(*this);
	}
}

Action Image::on_update() {
	auto& image_blueprint = std::get<ImageBlueprint>(blueprint->variant);
	if (image_blueprint.on_update) {
		auto[action, opt_sprite] = image_blueprint.on_update(*this);
		if (action.IsContinue() && opt_sprite) {
			_spriteType = *opt_sprite;
		}
		return std::move(action);
	} else {
		return MakeContinueAction();
	}
}

void Image::on_draw() {
	draw_background_color();
	if (_spriteType) {
		DrawSpriteOrTextLabel(M2_GAME.GetSpriteOrTextLabel(_spriteType), rect());
	}
	draw_border(rect(), vertical_border_width_px(), horizontal_border_width_px(), depressed ? SDL_Color{127, 127, 127, 255} : SDL_Color{255, 255, 255, 255});
}

void Image::set_sprite(const m2g::pb::SpriteType s) {
	_spriteType = s;
}
