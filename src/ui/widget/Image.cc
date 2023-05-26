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

Image::Image(const WidgetBlueprint* blueprint) : AbstractButton(blueprint), sprite(lookup_initial_sprite(blueprint)) {}

Action Image::update_content() {
	auto& image_blueprint = std::get<ImageBlueprint>(blueprint->variant);
	if (image_blueprint.update_callback) {
		auto[action, opt_sprite] = image_blueprint.update_callback();
		if (action == Action::CONTINUE && opt_sprite) {
			sprite =  lookup_sprite(*opt_sprite);
		}
		return action;
	} else {
		return Action::CONTINUE;
	}
}

void Image::draw() {
	draw_background_color(rect_px, blueprint->background_color);
	if (sprite) {
		// Make sure sprite is drawn square
		SDL_Rect dstrect;
		if (rect_px.h < rect_px.w) {
			dstrect = {
					.x = rect_px.x + (rect_px.w - rect_px.h) / 2,
					.y = rect_px.y,
					.w = rect_px.h,
					.h = rect_px.h
			};
		} else {
			dstrect = {
					.x = rect_px.x,
					.y = rect_px.y + (rect_px.h - rect_px.w) / 2,
					.w = rect_px.w,
					.h = rect_px.w
			};
		}
		auto srcrect = sdl::to_rect(sprite->sprite().rect());
		SDL_RenderCopy(GAME.renderer, sprite->sprite_sheet().texture(), &srcrect, &dstrect);
	}
	draw_border(rect_px, blueprint->border_width_px, depressed ? SDL_Color{127, 127, 127, 255} : SDL_Color{255, 255, 255, 255});
}
