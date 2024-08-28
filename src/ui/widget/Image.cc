#include <m2/ui/widget/Image.h>
#include <m2/Game.h>

using namespace m2;
using namespace m2::ui;
using namespace m2::ui::widget;

namespace {
	const Sprite* lookup_sprite(m2g::pb::SpriteType sprite_type) {
		return sprite_type ? &M2_GAME.get_sprite(sprite_type) : nullptr;
	}
}

Image::Image(State* parent, const WidgetBlueprint* blueprint) : AbstractButton(parent, blueprint) {
	_sprite = std::make_pair(image_blueprint().initial_sprite, lookup_sprite(image_blueprint().initial_sprite));

	if (image_blueprint().on_create) {
		image_blueprint().on_create(*this);
	}
}

Action Image::on_update() {
	auto& image_blueprint = std::get<ImageBlueprint>(blueprint->variant);
	if (image_blueprint.on_update) {
		auto[action, opt_sprite] = image_blueprint.on_update(*this);
		if (action.is_continue() && opt_sprite) {
			_sprite = std::make_pair(*opt_sprite, lookup_sprite(*opt_sprite));
		}
		return std::move(action);
	} else {
		return make_continue_action();
	}
}

void Image::on_draw() {
	draw_background_color(rect_px, blueprint->background_color);
	if (_sprite.second) {
		draw_sprite(*_sprite.second, rect_px);
	}
	draw_border(rect_px, vertical_border_width_px(), horizontal_border_width_px(), depressed ? SDL_Color{127, 127, 127, 255} : SDL_Color{255, 255, 255, 255});
}

void Image::set_sprite(m2g::pb::SpriteType s) {
	_sprite = std::make_pair(s, lookup_sprite(s));
}
