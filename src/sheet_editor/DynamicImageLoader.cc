#include <m2/sheet_editor/DynamicImageLoader.h>
#include <m2/game/object/Pixel.h>
#include <m2/Game.h>
#include <SDL2/SDL_image.h>

m2::expected<m2::DynamicImageLoader> m2::DynamicImageLoader::create(const std::filesystem::path& image_path) {
	// Load image
	sdl::SurfaceUniquePtr tmp_surface(IMG_Load(image_path.string().c_str()));
	if (!tmp_surface) {
		return make_unexpected("Unable to load image " + image_path.string() + ": " + IMG_GetError());
	}
	// Convert to a more conventional format
	sdl::SurfaceUniquePtr surface(SDL_ConvertSurfaceFormat(tmp_surface.get(), SDL_PIXELFORMAT_BGRA32, 0));
	if (!surface) {
		return make_unexpected("Unable to convert image format: " + std::string(SDL_GetError()));
	}

	return DynamicImageLoader{std::move(surface)};
}

m2::VecI m2::DynamicImageLoader::image_size() const {
	if (_image) {
		return VecI{_image->w, _image->h};
	} else {
		return VecI{};
	}
}

m2::ObjectId m2::DynamicImageLoader::load(const VecI& position) {
	// Lookup pixel
	SDL_LockSurface(_image.get());
	auto pixel = sdl::get_pixel(_image.get(), position.x, position.y);
	SDL_UnlockSurface(_image.get());
	if (not pixel) {
		return 0;
	}

	// Get SDL_Color from pixel
	SDL_Color color;
	SDL_GetRGBA(*pixel, _image->format, &color.r, &color.g, &color.b, &color.a);

	// Create pixel object
	return obj::create_pixel(static_cast<VecF>(position), color);
}

void m2::DynamicImageLoader::unload(ObjectId id) {
	M2_DEFER(create_object_deleter(id));
}
