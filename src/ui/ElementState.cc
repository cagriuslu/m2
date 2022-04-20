#include <m2/ui/ElementState.h>
#include <m2/Game.hh>
#include <SDL_render.h>

using namespace m2::ui;

ElementState::ElementState(const ElementBlueprint* blueprint) : blueprint(blueprint), rect_px({}) {}

void ElementState::update_position(const SDL_Rect &rect_px_) {
    this->rect_px = rect_px_;
}

Action ElementState::handle_events(const Events &events) {
    (void)events;
    return Action::CONTINUE;
}

Action ElementState::update_content() { return Action::CONTINUE; }

void ElementState::draw() {}

SDL_Texture* ElementState::generate_font_texture(const char* text) {
    SDL_Surface *textSurf = TTF_RenderUTF8_Blended(GAME.ttfFont, text, SDL_Color{255, 255, 255, 255});
    SDL_Texture *texture = SDL_CreateTextureFromSurface(GAME.sdlRenderer, textSurf);
    SDL_FreeSurface(textSurf);
    return texture;
}

void ElementState::draw_text(const SDL_Rect& rect, SDL_Texture& texture, TextAlignment align) {
    int text_w = 0, text_h = 0;
    SDL_QueryTexture(&texture, nullptr, nullptr, &text_w, &text_h);

    SDL_Rect dstrect;
    switch (align) {
        case TextAlignment::LEFT:
            dstrect = {
                    rect.x,
                    rect.y + rect.h / 2 - text_h / 2,
                    text_w,
                    text_h
            };
            break;
        case TextAlignment::RIGHT:
            dstrect = {
                    rect.x + rect.w - text_w,
                    rect.y + rect.h / 2 - text_h / 2,
                    text_w,
                    text_h
            };
            break;
        default:
            dstrect = {
                    rect.x + rect.w / 2 - text_w / 2,
                    rect.y + rect.h / 2 - text_h / 2,
                    text_w,
                    text_h
            };
            break;
    }
    SDL_RenderCopy(GAME.sdlRenderer, &texture, nullptr, &dstrect);
}
