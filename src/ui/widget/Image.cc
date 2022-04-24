#include <m2/ui/widget/Image.h>
#include <m2/ui/WidgetBlueprint.h>
#include <m2/ui/UI.h>
#include <m2/Game.hh>
#include <m2g/SpriteBlueprint.h>

m2::ui::wdg::ImageState::ImageState(const WidgetBlueprint* blueprint) : AbstractButtonState(blueprint), sprite_index(std::get<ImageBlueprint>(blueprint->variant).initial_sprite_index) {}

m2::ui::Action m2::ui::wdg::ImageState::update_content() {
    auto& image_blueprint = std::get<ImageBlueprint>(blueprint->variant);
    if (image_blueprint.update_callback) {
        auto[action, optional_rect] = image_blueprint.update_callback();
        if (action == Action::CONTINUE && optional_rect) {
	        sprite_index = *optional_rect;
        }
        return action;
    } else {
        return Action::CONTINUE;
    }
}

void m2::ui::wdg::ImageState::draw() {
	UIState::draw_background_color(rect_px, blueprint->background_color);
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
	SDL_RenderCopy(GAME.sdlRenderer, GAME.sdlTexture, &m2g::sprites[sprite_index].texture_rect, &dstrect);
	UIState::draw_border(rect_px, blueprint->border_width_px);
}
