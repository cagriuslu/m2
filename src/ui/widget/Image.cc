#include <m2/ui/widget/Image.h>
#include <m2/ui/WidgetBlueprint.h>
#include <m2/ui/UI.h>
#include <m2/Game.hh>
#include <m2g/SpriteBlueprint.h>
#include <m2/SDLUtils.hh>

namespace {
	const m2::Sprite* lookup_sprite(const std::string& key) {
		return &GAME.sprite_key_to_sprite_map.at(key);
	}

	const m2::Sprite* lookup_initial_sprite(const m2::ui::WidgetBlueprint *blueprint) {
		const auto& initial_sprite_key = std::get<m2::ui::wdg::ImageBlueprint>(blueprint->variant).initial_sprite_key;
		if (initial_sprite_key.empty()) {
			return nullptr;
		} else {
			return lookup_sprite(initial_sprite_key);
		}
	}
}

m2::ui::wdg::ImageState::ImageState(const WidgetBlueprint* blueprint) : AbstractButtonState(blueprint), sprite(lookup_initial_sprite(blueprint)) {}

m2::ui::Action m2::ui::wdg::ImageState::update_content() {
    auto& image_blueprint = std::get<ImageBlueprint>(blueprint->variant);
    if (image_blueprint.update_callback) {
        auto[action, opt_key] = image_blueprint.update_callback();
        if (action == Action::CONTINUE && opt_key) {
			sprite = lookup_sprite(*opt_key);
        }
        return action;
    } else {
        return Action::CONTINUE;
    }
}

void m2::ui::wdg::ImageState::draw() {
	UIState::draw_background_color(rect_px, blueprint->background_color);
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
		SDL_RenderCopy(GAME.sdlRenderer, sprite->sprite_sheet().texture(), &srcrect, &dstrect);
	}
	UIState::draw_border(rect_px, blueprint->border_width_px);
}
