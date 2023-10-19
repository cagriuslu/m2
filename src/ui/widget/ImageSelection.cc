#include <m2/ui/widget/ImageSelection.h>
#include <m2/ui/widget/Image.h>
#include <m2/Game.h>
#include <SDL2/SDL_image.h>

using namespace m2;
using namespace m2::ui;
using namespace m2::ui::widget;

ImageSelection::ImageSelection(State* parent, const WidgetBlueprint* blueprint) : Widget(parent, blueprint) {
	select(0);
}

Action ImageSelection::on_event(Events& events) {
	auto rect = RectI{rect_px};
	auto buttons_rect = rect.trim_top(rect.w);
	auto inc_button_rect = buttons_rect.trim_left(buttons_rect.w / 2);
	auto dec_button_rect = buttons_rect.trim_right(buttons_rect.w / 2);

	const auto& image_selection = std::get<ImageSelectionBlueprint>(blueprint->variant);

	if (!_inc_depressed && events.pop_mouse_button_press(MouseButton::PRIMARY, inc_button_rect)) {
		_inc_depressed = true;
		_dec_depressed = false;
	} else if (!_dec_depressed && events.pop_mouse_button_press(MouseButton::PRIMARY, dec_button_rect)) {
		_dec_depressed = true;
		_inc_depressed = false;
	} else if (_inc_depressed && events.pop_mouse_button_release(MouseButton::PRIMARY, inc_button_rect)) {
		_inc_depressed = false;
		if (_selection + 1 < image_selection.list.size()) {
			select(_selection + 1);
		}
	} else if (_dec_depressed && events.pop_mouse_button_release(MouseButton::PRIMARY, dec_button_rect)) {
		_dec_depressed = false;
		if (0 < _selection) {
			select(_selection - 1);
		}
	}
	return Action::CONTINUE;
}

Action ImageSelection::select(unsigned index) {
	_selection = index;

	const auto& image_selection = std::get<ImageSelectionBlueprint>(blueprint->variant);
	if (!image_selection.list.empty()) {
		const auto& action_callback = image_selection.on_action;
		if (action_callback) {
			return action_callback(*this);
		}
	}
	return Action::CONTINUE;
}

void ImageSelection::on_draw() {
	auto rect = RectI{rect_px};
	auto image_rect = rect.trim_bottom(rect.h - rect.w);
	auto buttons_rect = rect.trim_top(rect.w);
	auto inc_button_rect = buttons_rect.trim_left(buttons_rect.w / 2);
	auto inc_button_symbol_rect = inc_button_rect.trim_to_square().trim(5);
	auto dec_button_rect = buttons_rect.trim_right(buttons_rect.w / 2);
	auto dec_button_symbol_rect = dec_button_rect.trim_to_square().trim(5);

	draw_background_color(rect_px, blueprint->background_color);

	const auto& image_selection = std::get<ImageSelectionBlueprint>(blueprint->variant);
	if (!image_selection.list.empty()) {
		const auto& sprite = GAME.get_sprite(image_selection.list[_selection]);
		auto dst_rect = static_cast<SDL_Rect>(image_rect);
		draw_sprite(sprite, dst_rect);
	}

	static SDL_Texture* up_symbol = IMG_LoadTexture(GAME.renderer, "resource/up-symbol.svg");
	auto up_dstrect = (SDL_Rect)inc_button_symbol_rect;
	SDL_RenderCopy(GAME.renderer, up_symbol, nullptr, &up_dstrect);
	draw_border((SDL_Rect)inc_button_rect, blueprint->border_width_px);

	static SDL_Texture* down_symbol = IMG_LoadTexture(GAME.renderer, "resource/down-symbol.svg");
	auto down_dstrect = (SDL_Rect)dec_button_symbol_rect;
	SDL_RenderCopy(GAME.renderer, down_symbol, nullptr, &down_dstrect);
	draw_border((SDL_Rect)dec_button_rect, blueprint->border_width_px);

	draw_border(rect_px, blueprint->border_width_px);
}
