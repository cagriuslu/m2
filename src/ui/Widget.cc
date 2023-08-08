#include <m2/ui/Widget.h>
#include <m2/Game.h>

using namespace m2::ui;

Widget::Widget(const WidgetBlueprint* blueprint) : enabled(blueprint->initially_enabled), blueprint(blueprint) {}

void Widget::update_position(const SDL_Rect &rect_px_) {
	this->rect_px = rect_px_;
}

Action Widget::handle_events(MAYBE Events &events) {
	return Action::CONTINUE;
}

void Widget::focus_changed() {}

Action Widget::update_content() { return Action::CONTINUE; }

void Widget::draw() {}

void Widget::draw_background_color(const SDL_Rect& rect, const SDL_Color& color) {
	if (color.r || color.g || color.b || color.a) {
		SDL_SetRenderDrawColor(GAME.renderer, color.r, color.g, color.b, color.a);
		SDL_SetRenderDrawBlendMode(GAME.renderer, SDL_BLENDMODE_BLEND);
		SDL_RenderFillRect(GAME.renderer, &rect);
	}
}

void Widget::draw_text(const SDL_Rect& rect, SDL_Texture& texture, TextAlignment align) {
	int text_w = 0, text_h = 0;
	SDL_QueryTexture(&texture, nullptr, nullptr, &text_w, &text_h);

	// Current font has 2x5 letters
	int letter_w = 2;
	int letter_h = 5;
	// Current font is rendered with 280px
	if (text_h != 280) {
		throw M2FATAL("Unexpected font height");
	}
	if ((text_w % 112) != 0) {
		throw M2FATAL("Unexpected font aspect ratio");
	}
	int char_count = text_w / 112;

	// Do the math one order above
	auto rect_w_1000 = rect.w * 1000;
	auto rect_h_1000 = rect.h * 1000;
	// Decide whether to squeeze from the sides, or top and bottom
	auto height_multiplier_1000 = rect_h_1000 / text_h;
	auto ideal_width_1000 = text_w * height_multiplier_1000;
	int provisional_text_w, provisional_text_h;
	if (ideal_width_1000 < rect_w_1000) {
		// Rect is wider than the text
		provisional_text_w = text_w * height_multiplier_1000 / 1000;
		provisional_text_h = rect.h;
	} else {
		// Rect is taller than the text
		auto width_multiplier_1000 = rect_w_1000 / text_w;
		provisional_text_w = rect.w;
		provisional_text_h = text_h * width_multiplier_1000 / 1000;
	}
	// Make sure the width is an integer multiple of character count
	provisional_text_w /= char_count;
	provisional_text_w *= char_count;
	// Apply correction based on letter aspect ratio
	int final_text_w, final_text_h;
	int letter_scale_h_1000 = 1000 * provisional_text_h / letter_h;
	int letter_scale_w_1000 = 1000 * provisional_text_w / letter_w;
	if (letter_scale_h_1000 < letter_scale_w_1000) {
		// Height is correct, apply correction to width
		int expected_letter_w = letter_w * letter_scale_h_1000 / 1000;
		final_text_w = char_count * expected_letter_w;
		final_text_h = letter_h * letter_scale_h_1000 / 1000;
	} else {
		// Width is correct, apply correction to height
		int expected_letter_h = letter_h * letter_scale_w_1000 / 1000;
		final_text_w = letter_w * letter_scale_w_1000 / 1000;
		final_text_h = expected_letter_h;
	}

	SDL_Rect dstrect{};
	dstrect.y = rect.y + rect.h / 2 - final_text_h / 2;
	dstrect.w = final_text_w;
	dstrect.h = final_text_h;
	switch (align) {
		case TextAlignment::LEFT:
			dstrect.x = rect.x;
			break;
		case TextAlignment::RIGHT:
			dstrect.x = rect.x + rect.w - final_text_w;
			break;
		default:
			dstrect.x = rect.x + rect.w / 2 - final_text_w / 2;
			break;
	}
	SDL_RenderCopy(GAME.renderer, &texture, nullptr, &dstrect);
}

void Widget::draw_border(const SDL_Rect& rect, unsigned border_width_px, const SDL_Color& color) {
	if (border_width_px) {
		SDL_SetRenderDrawColor(GAME.renderer, color.r, color.g, color.b, color.a);
		SDL_SetRenderDrawBlendMode(GAME.renderer, SDL_BLENDMODE_BLEND);
		SDL_RenderDrawRect(GAME.renderer, &rect);
	}
}
