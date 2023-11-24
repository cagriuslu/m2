#pragma once

#include "Texture.h"
#include <m2/Meta.h>

namespace m2::sdl {
    class FontTexture {
        TextureUniquePtr _texture;
        std::string _text;
        std::variant<int, float, std::string> _value;

        FontTexture(SDL_Texture* texture, int number) : _texture(texture), _value(number) {}
        FontTexture(SDL_Texture* texture, float number) : _texture(texture), _value(number) {}
        FontTexture(SDL_Texture* texture, std::string text) : _texture(texture), _value(std::move(text)) {}
    public:
        FontTexture() = default;
        static expected<FontTexture> create(int number, SDL_Color color = {255, 255, 255, 255});
        static expected<FontTexture> create(float number, SDL_Color color = {255, 255, 255, 255});
        static expected<FontTexture> create(const std::string& text, SDL_Color color = {255, 255, 255, 255});

        [[nodiscard]] SDL_Texture& texture() const { return *_texture; }
        [[nodiscard]] int int_value() const { return std::get<int>(_value); }
        [[nodiscard]] float float_value() const { return std::get<float>(_value); }
        [[nodiscard]] std::string_view string_value() const { return std::get<std::string>(_value); }
        explicit operator bool() const { return static_cast<bool>(_texture); }
    };
}
