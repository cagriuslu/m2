#include <m2/ui/widget/Image.h>
#include <m2/Game.h>

using namespace m2;
using namespace m2::ui;
using namespace m2::ui::widget;

namespace {
	const Sprite* lookup_sprite(m2g::pb::SpriteType sprite_type) {
		return sprite_type ? &GAME.get_sprite(sprite_type) : nullptr;
	}

	const Sprite* lookup_initial_sprite(const WidgetBlueprint *blueprint) {
		return lookup_sprite(std::get<ImageBlueprint>(blueprint->variant).initial_sprite);
	}
}

Image::Image(State* parent, const WidgetBlueprint* blueprint) : AbstractButton(parent, blueprint), _sprite(lookup_initial_sprite(blueprint)) {}

Action Image::on_update() {
	auto& image_blueprint = std::get<ImageBlueprint>(blueprint->variant);
	if (image_blueprint.on_update) {
		auto[action, opt_sprite] = image_blueprint.on_update(*this);
		if (action == Action::CONTINUE && opt_sprite) {
			_sprite =  lookup_sprite(*opt_sprite);
		}
		return action;
	} else {
		return Action::CONTINUE;
	}
}

void Image::on_draw() {
	draw_background_color(rect_px, blueprint->background_color);
	if (_sprite) {
		draw_sprite(*_sprite, rect_px);
	}
	draw_border(rect_px, blueprint->border_width_px, depressed ? SDL_Color{127, 127, 127, 255} : SDL_Color{255, 255, 255, 255});
}
