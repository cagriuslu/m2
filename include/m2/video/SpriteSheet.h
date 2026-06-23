#pragma once
#include <m2/thirdparty/video/Renderer.h>
#include <m2/thirdparty/video/Surface.h>
#include <m2/thirdparty/video/Texture.h>
#include <optional>
#include <Sprite.pb.h>

namespace m2 {
	class SpriteSheet final {
		pb::SpriteSheet _pb;
		thirdparty::video::Surface _surface;
		std::optional<thirdparty::video::Texture> _texture;

		SpriteSheet(const pb::SpriteSheet& spriteSheet, thirdparty::video::Renderer& renderer);

	public:
		static std::vector<SpriteSheet> LoadSpriteSheets(const pb::SpriteSheets& spriteSheets, thirdparty::video::Renderer& renderer);

		// Accessors

		[[nodiscard]] const pb::SpriteSheet& Pb() const { return _pb; }
		[[nodiscard]] const thirdparty::video::Surface& surface() const { return _surface; }
		[[nodiscard]] const thirdparty::video::Texture& texture() const { return *_texture; }
	};
}
