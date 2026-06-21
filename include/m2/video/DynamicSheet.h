#pragma once
#include <m2/math/RectI.h>
#include <m2/sdl/Surface.h>
#include <m2/thirdparty/video/Texture.h>
#include <optional>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>

namespace m2 {
	/// Sprite sheet that dynamically increases in size on demand
	class DynamicSheet {
		SDL_Renderer* _renderer;
		sdl::SurfaceUniquePtr _surface;
		std::optional<thirdparty::video::Texture> _texture;
		int _lastW{}, _lastH{}, _heightOfCurrentRow{};

	public:
		explicit DynamicSheet(SDL_Renderer* renderer);

		// Accessors

		[[nodiscard]] const thirdparty::video::Texture& Texture() const { return *_texture; }

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
