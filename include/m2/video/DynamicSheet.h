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
		SDL_Renderer* _renderer;
		bool _darkenForLightning;
		sdl::SurfaceUniquePtr _surface;
		sdl::TextureUniquePtr _texture;
		int _lastW{}, _lastH{}, _heightOfCurrentRow{};

	public:
		DynamicSheet(SDL_Renderer* renderer, bool darkenForLightning);

		// Accessors

		[[nodiscard]] SDL_Texture* Texture() const { return _texture.get(); }

		// Modifiers

		/// Tries to allocate the requested area on the dynamic surface, optionally locks the surface, and calls the
		/// mutator with the surface and the area. The same area is returned at the end. Surface must be locked only if
		/// the raw pixels will be mutated. For blitting and similar operations, the surface shouldn't be locked.
		expected<RectI> AllocateAndMutate(int requestedW, int requestedH, const std::function<void(SDL_Surface*,const RectI&)>& mutator, bool lockSurface = true);

	protected:
		[[nodiscard]] int Width() const { return _surface->w; }
		[[nodiscard]] int Height() const { return _surface->h; }
	};
}
