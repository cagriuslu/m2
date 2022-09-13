#include <m2/ui/WidgetState.h>
#include <m2/Game.hh>
#include <SDL_render.h>

using namespace m2::ui;

WidgetState::WidgetState(const WidgetBlueprint* blueprint) : blueprint(blueprint), rect_px({}) {}

void WidgetState::update_position(const SDL_Rect &rect_px_) {
    this->rect_px = rect_px_;
}

Action WidgetState::handle_events(Events &events) {
    (void)events;
    return Action::CONTINUE;
}

Action WidgetState::update_content() { return Action::CONTINUE; }

void WidgetState::draw() {}

SDL_Texture* WidgetState::generate_font_texture(const char* text) {
    SDL_Surface *textSurf = TTF_RenderUTF8_Blended(GAME.ttfFont, text, SDL_Color{255, 255, 255, 255});
    SDL_Texture *texture = SDL_CreateTextureFromSurface(GAME.sdlRenderer, textSurf);
    SDL_FreeSurface(textSurf);
    return texture;
}

void WidgetState::draw_text(const SDL_Rect& rect, SDL_Texture& texture, TextAlignment align) {
    int text_w = 0, text_h = 0;
    SDL_QueryTexture(&texture, nullptr, nullptr, &text_w, &text_h);

    SDL_Rect dstrect;
	dstrect.y = rect.h < text_h ? rect.y : rect.y + rect.h / 2 - text_h / 2;
	dstrect.w = rect.w < text_w ? rect.w : text_w;
	dstrect.h = rect.h < text_h ? rect.h : text_h;
    switch (align) {
        case TextAlignment::LEFT:
			dstrect.x = rect.x;
            break;
        case TextAlignment::RIGHT:
			dstrect.x = rect.w < text_w ? rect.x : rect.x + rect.w - text_w;
            break;
        default:
			dstrect.x = rect.w < text_w ? rect.x : rect.x + rect.w / 2 - text_w / 2;
            break;
    }
    SDL_RenderCopy(GAME.sdlRenderer, &texture, nullptr, &dstrect);
}

std::unique_ptr<WidgetState> m2::ui::create_widget_instance(const WidgetBlueprint& blueprint) {
	using namespace wdg;
	std::unique_ptr<WidgetState> state;
	if (std::holds_alternative<NestedUIBlueprint>(blueprint.variant)) {
		state = std::make_unique<NestedUIState>(&blueprint);
	} else if (std::holds_alternative<TextBlueprint>(blueprint.variant)) {
		state = std::make_unique<TextState>(&blueprint);
	} else if (std::holds_alternative<TextInputBlueprint>(blueprint.variant)) {
		state = std::make_unique<TextInputState>(&blueprint);
	} else if (std::holds_alternative<ImageBlueprint>(blueprint.variant)) {
		state = std::make_unique<ImageState>(&blueprint);
	} else if (std::holds_alternative<ProgressBarBlueprint>(blueprint.variant)) {
		state = std::make_unique<ProgressBarState>(&blueprint);
	} else {
		throw M2FATAL("Implementation");
	}
	return state;
}
