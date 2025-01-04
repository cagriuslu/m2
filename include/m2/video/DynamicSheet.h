#pragma once
#include <m2/math/RectI.h>
#include <m2/sdl/Surface.h>
#include <m2/sdl/Texture.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <memory>

namespace m2 {
	/// Sprite sheet that dynamically increases in size on demand
	class DynamicSheet {
		SDL_Renderer* _renderer; // TODO Use weak-ptr instead
		std::unique_ptr<SDL_Surface, sdl::SurfaceDeleter> _surface;
		int _lastW{}, _lastH{}, _heightOfCurrentRow{};
		std::unique_ptr<SDL_Texture, sdl::TextureDeleter> _texture;

	public:
		explicit DynamicSheet(SDL_Renderer* renderer);

		// Accessors

		[[nodiscard]] SDL_Texture* Texture() const;

		// Modifiers

		std::pair<SDL_Surface*, RectI> Alloc(int requestedW, int requestedH);
		SDL_Texture* RecreateTexture(bool lightning);

	protected:
		[[nodiscard]] int Width() const { return _surface->w; }
		[[nodiscard]] int Height() const { return _surface->h; }
	};
}
