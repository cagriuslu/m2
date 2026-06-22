#pragma once
#include <m2/video/Color.h>
#include <m2/math/VecI.h>
#include <filesystem>
#include <string>

typedef struct _TTF_Font TTF_Font;

namespace m2::thirdparty::video {
	class Surface {
		void* _surface{};

		explicit Surface(void* surface) : _surface(surface) {}

	public:
		static Surface CreateFromImageFile(const std::filesystem::path& imageFilePath);
		static Surface CreateBlank(int w, int h, uint32_t pixelFormat);
		static Surface RenderTextSolid(TTF_Font* font, const std::string& text, const RGBA& color);
		static Surface RenderTextBlended(TTF_Font* font, const std::string& text, const RGBA& color);
		static Surface RenderTextBlendedWrapped(TTF_Font* font, const std::string& text, const RGBA& color, int wrapWidthPx);

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
	};
}
