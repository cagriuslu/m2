#include <SDL2/SDL_image.h>
#include <m2/Game.h>
#include <m2/bulk_sheet_editor/DynamicSpriteSheetLoader.h>
#include <m2/game/object/TexturePatch.h>

m2::expected<m2::DynamicSpriteSheetLoader> m2::DynamicSpriteSheetLoader::create(
    const std::filesystem::path& image_path, int image_ppm) {
	// Load image
	sdl::SurfaceUniquePtr surface(IMG_Load(image_path.c_str()));
	if (!surface) {
		return make_unexpected("Unable to load image " + image_path.string() + ": " + IMG_GetError());
	}
	sdl::TextureUniquePtr texture(SDL_CreateTextureFromSurface(GAME.renderer, surface.get()));
	if (!texture) {
		return make_unexpected("Unable to create texture from surface: " + std::string(SDL_GetError()));
	}
	return DynamicSpriteSheetLoader{std::move(surface), std::move(texture), image_ppm};
}

m2::VecI m2::DynamicSpriteSheetLoader::image_size() const {
	if (_surface) {
		return VecI{_surface->w, _surface->h};
	} else {
		return VecI{};
	}
}

m2::ObjectId m2::DynamicSpriteSheetLoader::load(const VecI& position) {
	if (position.x < 0 || position.y < 0) {
		return 0;
	}
	auto image_sz = image_size();
	if (image_sz.x / _image_ppm < position.x || image_sz.y / _image_ppm < position.y) {
		return 0;
	}

	return obj::create_texture_patch(
	    static_cast<VecF>(position), _texture.get(),
	    RectI{position.x * _image_ppm, position.y * _image_ppm, _image_ppm, _image_ppm}, _image_ppm);
}

void m2::DynamicSpriteSheetLoader::unload(ObjectId id) { GAME.add_deferred_action(create_object_deleter(id)); }
