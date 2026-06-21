#pragma once
#include <m2/sdl/Surface.h>
#include <m2/thirdparty/video/Texture.h>
#include <optional>
#include <Sprite.pb.h>

namespace m2 {
	class SpriteSheet final {
		pb::SpriteSheet _pb;
		sdl::SurfaceUniquePtr _surface;
		std::optional<thirdparty::video::Texture> _texture;

		SpriteSheet(const pb::SpriteSheet& spriteSheet, SDL_Renderer* renderer);

	public:
		static std::vector<SpriteSheet> LoadSpriteSheets(const pb::SpriteSheets& spriteSheets, SDL_Renderer* renderer);

		// Accessors

		[[nodiscard]] const pb::SpriteSheet& Pb() const { return _pb; }
		[[nodiscard]] SDL_Surface* surface() const { return _surface.get(); }
		[[nodiscard]] const thirdparty::video::Texture& texture() const { return *_texture; }
	};
}
