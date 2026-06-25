#pragma once
#include "m2/common/Meta.h"
#include <m2/common/math/VecF.h>
#include <m2/common/math/VecI.h>
#include <m2/common/video/Color.h>
#include <span>

namespace m2::thirdparty::video {
	class Renderer {
		void* _renderer{};

		explicit Renderer(void* renderer) : _renderer(renderer) {}

	public:
		static expected<Renderer> Create(void* sdlWindow);

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept;
		Renderer& operator=(Renderer&&) noexcept;
		virtual ~Renderer();

		[[nodiscard]] void* RawHandle() const { return _renderer; } // TODO remove
		[[nodiscard]] VecI GetOutputSize() const;
		[[nodiscard]] std::string GetName() const;

		/// Sets the color used by subsequent clearing and primitive drawing operations.
		void SetDrawColor(const RGBA& color);
		/// Clears the whole render target using the current draw color.
		void Clear();
		/// Presents the back buffer, making everything drawn since the last present visible on the window.
		void Present();

		/// Draws a connected strip of line segments through the given screen-pixel points using the given color.
		void DrawLineStrip(std::span<const VecF> pointsPx, const RGBA& color); // TODO move to Shapes
	};
}
