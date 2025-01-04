#pragma once
#include <m2/sdl/Surface.h>
#include <m2/sdl/Texture.h>
#include <Sprite.pb.h>

namespace m2 {
	class SpriteSheet final {
		pb::SpriteSheet _pb;
		std::unique_ptr<SDL_Surface, sdl::SurfaceDeleter> _surface;
		std::unique_ptr<SDL_Texture, sdl::TextureDeleter> _texture;

	public:
		SpriteSheet(const pb::SpriteSheet& spriteSheet, SDL_Renderer* renderer, bool lightning);
		static std::vector<SpriteSheet> LoadSpriteSheets(const pb::SpriteSheets& spriteSheets, SDL_Renderer* renderer, bool lightning);

		// Accessors

		[[nodiscard]] const pb::SpriteSheet& Pb() const { return _pb; }
		[[nodiscard]] SDL_Surface* surface() const { return _surface.get(); }
		[[nodiscard]] SDL_Texture* texture() const { return _texture.get(); }
	};
}
