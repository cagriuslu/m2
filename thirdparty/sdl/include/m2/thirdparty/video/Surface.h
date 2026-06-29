#pragma once
#include <m2/common/Meta.h>
#include <m2/common/video/Color.h>
#include <m2/common/math/VecI.h>
#include <m2/common/math/RectI.h>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

#include <m2/thirdparty/video/Font.h>

namespace m2::thirdparty::video {
	class Surface {
		void* _surface{};

		explicit Surface(void* surface) : _surface(surface) {}

	public:
		static Surface CreateFromImageFile(const std::filesystem::path& imageFilePath);
		static Surface CreateBlank(int w, int h, uint32_t pixelFormat);
		static Surface CreateBlankWithAlpha(int w, int h);
		static Surface RenderTextSolid(const Font& font, const std::string& text, const RGBA& color);
		static Surface RenderTextBlended(const Font& font, const std::string& text, const RGBA& color);
		static Surface RenderTextBlendedWrapped(const Font& font, const std::string& text, const RGBA& color, int wrapWidthPx);

		/// Copy not allowed
		Surface(const Surface&) = delete;
		Surface& operator=(const Surface&) = delete;
		/// Move allowed
		Surface(Surface&&) noexcept;
		Surface& operator=(Surface&&) noexcept;
		/// Destructor
		virtual ~Surface();

		[[nodiscard]] void* RawHandle() const { return _surface; } // TODO remove
		[[nodiscard]] VecI Dimensions() const;
		[[nodiscard]] uint32_t PixelFormat() const;
		[[nodiscard]] int BytesPerPixel() const;

		/// Blits (copies) the given source rect of the source surface into the destination rect of this surface.
		/// If sourceRect is not given, the whole source surface is blitted. Blitting does not scale; only the position
		/// of the destination rect is used. Modifies this surface.
		expected<void> Blit(const Surface& source, std::optional<RectI> sourceRect, const RectI& destinationRect);

		/// Locks/unlocks the surface for direct pixel access. Treated as a logically-const operation.
		void Lock() const;
		void Unlock() const;

		/// Direct pixel access. The surface must be locked. Assumes 4 bytes-per-pixel and width-stride addressing.
		[[nodiscard]] RGBA GetPixel(int x, int y) const;
		void SetPixel(int x, int y, const RGBA& color);
	};
}
