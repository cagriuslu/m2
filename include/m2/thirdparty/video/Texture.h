#pragma once
#include <m2/video/Color.h>
#include <m2/math/RectI.h>

namespace m2::thirdparty::video {
	class Texture {
		void* _texture{};

		explicit Texture(void* texture) : _texture(texture) {}

	public:
		static Texture Generate(int w, int h, const std::function<RGBA(int x, int y)>&);

		// Copy not allowed
		Texture(const Texture& other) = delete;
		Texture& operator=(const Texture& other) = delete;
		// Move allowed
		Texture(Texture&&) noexcept;
		Texture& operator=(Texture&&) noexcept;
		// Destructor
		virtual ~Texture();

		/// Draws the texture to the screen.
		/// \param patchRect Portion of the texture to draw
		/// \param scale Pixel scale. 1 source pixel occupies (scale) pixels at the destination.
		/// \param positionOnScreen Position of the center of the texture patch on the screen, in screen coordinates.
		void Draw2d(const RectI* patchRect, float scale, const VecF& positionOnScreen) const;
	};
}
