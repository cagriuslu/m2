#pragma once
#include <m2/common/video/Color.h>
#include <m2/common/math/RectI.h>
#include <m2/common/math/VecI.h>
#include <m2/common/math/VecF.h>
#include <filesystem>
#include <functional>
#include <span>

namespace m2::thirdparty::video {
	class Renderer;

	class Texture {
		void* _texture{};

		explicit Texture(void* texture) : _texture(texture) {}

	public:
		static Texture Generate(Renderer& renderer, uint32_t pixelFormat, int w, int h, const std::function<RGBA(int x, int y)>&);
		static Texture CreateTargetableWindowSized(Renderer& renderer, uint32_t pixelFormat);
		static Texture CaptureWindow(Renderer& renderer, uint32_t pixelFormat);
		static Texture CreateFromImageFile(Renderer& renderer, const std::filesystem::path& imageFilePath);
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
		[[nodiscard]] VecF Dimensions() const;

		/// Sets this texture as the render target, runs `draw`, then restores the previous render target.
		void DrawOnto(Renderer& renderer, const std::function<void()>& draw);

		/// Copies this texture over the whole current render target (the window).
		void RenderToWindow(Renderer& renderer) const;
		void Render(Renderer& renderer, const RectF& destination) const;
		void Render(Renderer& renderer, const RectI& sourceRect, const RectF& destination) const;
		void RenderWithColorMod(Renderer& renderer, const RectF& destination, const RGB& mod) const;
		void Render(Renderer& renderer, const RectI& sourceRect, const RectF& destination, double angleDegrees, const VecI& rotationCenter) const;
		void RenderGeometry(Renderer& renderer, std::span<const VecF> positions, std::span<const VecF> texCoords, std::span<const int> indices) const;

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
