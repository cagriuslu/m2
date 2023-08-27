#pragma once

#include "../game/SpatialObjectLoader.h"

namespace m2 {
	class DynamicImageLoader : public SpatialObjectLoader {
		sdl::SurfaceUniquePtr _image;

	public:
		inline explicit DynamicImageLoader(sdl::SurfaceUniquePtr&& image) : _image(std::move(image)) {}
		static expected<DynamicImageLoader> create(const std::filesystem::path& image_path);

	protected:
		ObjectId load(const VecI& position) override;
		void unload(ObjectId id) override;
	};
}
