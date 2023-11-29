#pragma once

#include "../game/SpatialObjectLoader.h"

namespace m2 {
	class DynamicImageLoader final : public SpatialObjectLoader {
		sdl::SurfaceUniquePtr _image;

	public:
		explicit DynamicImageLoader(sdl::SurfaceUniquePtr&& image) : _image(std::move(image)) {}
		static expected<DynamicImageLoader> create(const std::filesystem::path& image_path);

		[[nodiscard]] VecI image_size() const;

	protected:
		ObjectId load(const VecI& position) override;
		void unload(ObjectId id) override;
	};
}
