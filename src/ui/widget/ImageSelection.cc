#include <m2/Game.h>
#include <m2/ui/widget/Image.h>
#include <m2/ui/widget/ImageSelection.h>

using namespace m2;
using namespace m2::ui;
using namespace m2::ui::widget;

ImageSelection::ImageSelection(State* parent, const WidgetBlueprint* blueprint)
    : Widget(parent, blueprint),
      _plus_texture(m2_move_or_throw_error(sdl::FontTexture::create(GAME.font, GAME.renderer, "+"))),
      _minus_texture(m2_move_or_throw_error(sdl::FontTexture::create(GAME.font, GAME.renderer, "-"))) {
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
	} else {
		// Check if scrolled
		if (auto scroll_amount = events.pop_mouse_wheel_vertical_scroll(rect); 0 < scroll_amount) {
			auto min_scroll_amount =
			    std::min(static_cast<size_t>(scroll_amount), image_selection.list.size() - _selection - 1);
			if (min_scroll_amount) {
				return select(_selection + min_scroll_amount);
			}
		} else if (scroll_amount < 0) {
			auto min_scroll_amount = std::min(static_cast<unsigned>(-scroll_amount), _selection);
			if (min_scroll_amount) {
				return select(_selection - min_scroll_amount);
			}
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
	draw_background_color(rect_px, blueprint->background_color);

	const auto& image_selection = std::get<ImageSelectionBlueprint>(blueprint->variant);
	if (!image_selection.list.empty()) {
		const auto& sprite = GAME.get_sprite(image_selection.list[_selection]);
		auto image_rect = rect_px.trim_bottom(rect_px.h - rect_px.w);
		draw_sprite(sprite, image_rect);
	}

	auto buttons_rect = rect_px.trim_top(rect_px.w);
	auto inc_button_rect = buttons_rect.trim_left(buttons_rect.w / 2);
	draw_text(inc_button_rect, *_plus_texture.texture(), TextAlignment::CENTER);
	draw_border(inc_button_rect, blueprint->border_width_px);

	auto dec_button_rect = buttons_rect.trim_right(buttons_rect.w / 2);
	draw_text(dec_button_rect, *_minus_texture.texture(), TextAlignment::CENTER);
	draw_border(dec_button_rect, blueprint->border_width_px);

	draw_border(rect_px, blueprint->border_width_px);
}
