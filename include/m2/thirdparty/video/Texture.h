#pragma once
#include <m2/video/Color.h>
#include <m2/math/RectI.h>
#include <m2/math/VecI.h>
#include <m2/math/VecF.h>
#include <filesystem>
#include <functional>
#include <span>

namespace m2::thirdparty::video {
	class Renderer;

	class Texture {
		void* _texture{};

		explicit Texture(void* texture) : _texture(texture) {}

	public:
		static Texture Generate(int w, int h, const std::function<RGBA(int x, int y)>&);
		static Texture CreateTargetableWindowSized();
		static Texture CaptureWindow();
		static Texture CreateFromImageFile(const std::filesystem::path& imageFilePath);
		static Texture AdoptRawTexture(void* rawSdlTexture);
		static Texture CreateFromSurface(Renderer& renderer, void* sdlSurface, bool linearFilter = false);

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
		void Render(const RectI& sourceRect, const RectI& destinationRect) const;
		void RenderWithColorMod(const RectI& destinationPx, const RGB& mod) const;
		void Render(const RectI& sourceRect, const RectI& destinationRect, double angleDegrees, const VecI& rotationCenter) const;
		void RenderGeometry(std::span<const VecF> positionsPx, std::span<const VecF> texCoords, std::span<const int> indices) const;

		class [[nodiscard]] ColorModGuard {
			void* _texture{};
			friend class Texture;
			ColorModGuard(void* texture, const RGB& mod);
		public:
			ColorModGuard() = default;
			ColorModGuard(const ColorModGuard&) = delete;
			ColorModGuard& operator=(const ColorModGuard&) = delete;
			ColorModGuard(ColorModGuard&&) noexcept;
			ColorModGuard& operator=(ColorModGuard&&) noexcept;
			~ColorModGuard();
			[[nodiscard]] explicit operator bool() const { return _texture != nullptr; }
		};
		[[nodiscard]] ColorModGuard ScopedColorMod(const RGB& mod) const;
	};
}
