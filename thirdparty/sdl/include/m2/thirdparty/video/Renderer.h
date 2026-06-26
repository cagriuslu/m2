#pragma once
#include "m2/common/Meta.h"
#include <m2/common/math/VecF.h>
#include <m2/common/math/VecI.h>
#include <m2/common/video/Color.h>
#include <span>

namespace m2::thirdparty::video {
	class Window;

	class Renderer {
		void* _window;
		void* _renderer;

		friend Window;
		explicit Renderer(void* window, void* renderer) : _window(window), _renderer(renderer) {}

	public:
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept;
		Renderer& operator=(Renderer&&) noexcept;
		virtual ~Renderer();

		[[nodiscard]] void* RawHandle() const { return _renderer; } // TODO remove
		/// Returns the number of pixels in both axes that correspond to unit distance in window coordinates
		[[nodiscard]] VecF GetPixelsPerWindowUnit() const;
		[[nodiscard]] std::string GetName() const;

		/// Sets the color used by subsequent clearing and primitive drawing operations.
		void SetDrawColor(const RGBA& color);
		/// Clears the whole render target using the current draw color.
		void Clear();
		/// Presents the back buffer, making everything drawn since the last present visible on the window.
		void Present();

		/// Draws a connected strip of line segments through the given window coordinate points using the given color.
		void DrawLineStrip(std::span<const VecF> points, const RGBA& color); // TODO move to Shapes
	};
}
