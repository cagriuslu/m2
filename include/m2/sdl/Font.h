#pragma once

#include <SDL2/SDL_ttf.h>
#include <m2/Meta.h>

#include "../VecI.h"
#include "Texture.h"

namespace m2::sdl {
	class FontTexture {
		TextureUniquePtr _texture;
		std::variant<int, float, std::string> _value;
		int _w{}, _h{};

		FontTexture(SDL_Texture* texture, int number, int w, int h) : _texture(texture), _value(number), _w(w), _h(h) {}
		FontTexture(SDL_Texture* texture, float number, int w, int h)
		    : _texture(texture), _value(number), _w(w), _h(h) {}
		FontTexture(SDL_Texture* texture, std::string text, int w, int h)
		    : _texture(texture), _value(std::move(text)), _w(w), _h(h) {}

	   public:
		FontTexture() = default;
		static expected<FontTexture> create(
		    TTF_Font* font, SDL_Renderer* renderer, int number, SDL_Color color = {255, 255, 255, 255});
		static expected<FontTexture> create(
		    TTF_Font* font, SDL_Renderer* renderer, float number, SDL_Color color = {255, 255, 255, 255});
		static expected<FontTexture> create(
		    TTF_Font* font, SDL_Renderer* renderer, const std::string& text, SDL_Color color = {255, 255, 255, 255});

		// Can be null if the string is empty
		[[nodiscard]] SDL_Texture* texture() const { return _texture.get(); }
		explicit operator bool() const { return texture(); }

		[[nodiscard]] int int_value() const { return std::get<int>(_value); }
		[[nodiscard]] float float_value() const { return std::get<float>(_value); }
		[[nodiscard]] std::string_view string_value() const { return std::get<std::string>(_value); }

		[[nodiscard]] int w() const { return _w; }
		[[nodiscard]] int h() const { return _h; }
	};
}  // namespace m2::sdl
