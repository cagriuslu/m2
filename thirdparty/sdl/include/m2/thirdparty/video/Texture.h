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
		/// Creates a targetable texture of the given size, using an alpha-capable pixel format so that what is drawn
		/// onto it can hold transparency.
		static Texture CreateTargetableWithAlpha(Renderer& renderer, int w, int h, bool linearFilter = false);
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

		/// Sets this texture as the render target, runs `draw`, then restores the previous render target. The previous
		/// render target is restored even if `draw` throws.
		void DrawOnto(Renderer& renderer, const std::function<void()>& draw);

		/// Copies this texture over the current viewport of the current render target. With no viewport set that is the
		/// whole target, which for the window target is the whole window.
		void RenderOverViewport(Renderer& renderer) const;
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

		enum class BlendMode {
			NONE,
			BLEND,
			/// PREMULTIPLIED is the mode for content whose color channels are already multiplied by their alpha.
			PREMULTIPLIED
		};

		/// Sets the blend mode used when this texture is drawn, and restores the previous one on scope exit.
		class [[nodiscard]] BlendModeGuard {
			friend class Texture;
			void* _texture{};
			BlendMode _previousBlendMode{};
			BlendModeGuard(void* texture, BlendMode mode);
		public:
			BlendModeGuard() = default;
			BlendModeGuard(const BlendModeGuard&) = delete;
			BlendModeGuard& operator=(const BlendModeGuard&) = delete;
			BlendModeGuard(BlendModeGuard&&) noexcept;
			BlendModeGuard& operator=(BlendModeGuard&&) noexcept;
			~BlendModeGuard();
			[[nodiscard]] explicit operator bool() const { return _texture != nullptr; }
		};
		[[nodiscard]] BlendModeGuard ScopedBlendMode(BlendMode mode) const;
	};
}
