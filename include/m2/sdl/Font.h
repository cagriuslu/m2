#pragma once

#include "Texture.h"
#include <m2/Meta.h>

namespace m2::sdl {
    class FontTexture {
        TextureUniquePtr _texture;
        std::string _text;

        FontTexture(SDL_Texture* texture, std::string  text) : _texture(texture), _text(std::move(text)) {}
    public:
        FontTexture() = default;
        static expected<FontTexture> create(const std::string& text, SDL_Color color = {255, 255, 255, 255});
        [[nodiscard]] SDL_Texture& texture() const { return *_texture; }
        [[nodiscard]] std::string_view text() const { return _text; }
        explicit operator bool() const { return static_cast<bool>(_texture); }
    };
}
