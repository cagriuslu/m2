#pragma once

#include <m2/sdl/Surface.h>

#include "../game/SpatialObjectLoader.h"

namespace m2 {
	class DynamicSpriteSheetLoader final : public SpatialObjectLoader {
		sdl::SurfaceUniquePtr _surface;
		sdl::TextureUniquePtr _texture;
		int _image_ppm;

	   public:
		explicit DynamicSpriteSheetLoader(
		    sdl::SurfaceUniquePtr&& surface, sdl::TextureUniquePtr&& texture, int image_ppm)
		    : _surface(std::move(surface)), _texture(std::move(texture)), _image_ppm(image_ppm) {}
		static expected<DynamicSpriteSheetLoader> create(const std::filesystem::path& image_path, int image_ppm);

		[[nodiscard]] VecI image_size() const;

	   protected:
		ObjectId load(const VecI& position) override;
		void unload(ObjectId id) override;
	};
}  // namespace m2
