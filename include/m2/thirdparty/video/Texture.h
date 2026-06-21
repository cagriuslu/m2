#pragma once
#include <m2/video/Color.h>
#include <m2/math/RectI.h>
#include <m2/math/VecI.h>
#include <filesystem>
#include <functional>

namespace m2::thirdparty::video {
	class Texture {
		void* _texture{};

		explicit Texture(void* texture) : _texture(texture) {}

	public:
		static Texture Generate(int w, int h, const std::function<RGBA(int x, int y)>&);
		static Texture CreateTargetableWindowSized();
		static Texture CaptureWindow();
		static Texture CreateFromImageFile(const std::filesystem::path& imageFilePath);
		static Texture AdoptRawTexture(void* rawSdlTexture);
		static Texture CreateFromSurface(void* sdlRenderer, void* sdlSurface);

		/// Copy not allowed
		Texture(const Texture& other) = delete;
		Texture& operator=(const Texture& other) = delete;
		/// Move allowed
		Texture(Texture&&) noexcept;
		Texture& operator=(Texture&&) noexcept;
		/// Destructor
		virtual ~Texture();

		[[nodiscard]] void* RawHandle() const { return _texture; } // TODO remove this, this class should do the drawing instead
		[[nodiscard]] VecI Dimensions() const;

		/// Sets this texture as the render target, runs `draw`, then restores the previous render target.
		void DrawOnto(const std::function<void()>& draw);

		/// Copies this texture over the whole current render target (the window).
		void RenderToWindow() const;
		void Render(const RectI& destinationPx) const;
		void RenderWithColorMod(const RectI& destinationPx, const RGB& mod) const;
	};
}
