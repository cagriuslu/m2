#include <m2/ui/ElementState.h>
#include <m2/Game.hh>
#include <SDL_render.h>

m2::ui::ElementState::ElementState(const ElementBlueprint* blueprint) : blueprint(blueprint), rect_px({}) {}

void m2::ui::ElementState::update_position(const SDL_Rect &rect_px_) {
    this->rect_px = rect_px_;
}

m2::ui::Action m2::ui::ElementState::update_content() { return Action::CONTINUE; }

void m2::ui::ElementState::draw() {}

SDL_Scancode m2::ui::ElementState::get_keyboard_shortcut() const {
	return SDL_SCANCODE_UNKNOWN;
}

void m2::ui::ElementState::set_depressed(bool state) { (void)state; }

m2::ui::Action m2::ui::ElementState::action() { return Action::CONTINUE; }

SDL_Texture* m2::ui::ElementState::generate_font_texture(const char* text) {
    SDL_Surface *textSurf = TTF_RenderUTF8_Blended(GAME.ttfFont, text, SDL_Color{255, 255, 255, 255});
    SDL_Texture *texture = SDL_CreateTextureFromSurface(GAME.sdlRenderer, textSurf);
    SDL_FreeSurface(textSurf);
    return texture;
}

void m2::ui::ElementState::draw_text(const SDL_Rect& rect, SDL_Texture& texture) {
    int text_w = 0, text_h = 0;
    SDL_QueryTexture(&texture, nullptr, nullptr, &text_w, &text_h);
    auto text_rect = SDL_Rect{
        rect.x + rect.w / 2 - text_w / 2,
        rect.y + rect.h / 2 - text_h / 2,
        text_w,
        text_h
    };
    SDL_RenderCopy(GAME.sdlRenderer, &texture, nullptr, &text_rect);
}
