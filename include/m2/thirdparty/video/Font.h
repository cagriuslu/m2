#pragma once
#include <m2/video/TextAlignment.h>
#include <filesystem>

namespace m2::thirdparty::video {
	class Font {
		void* _font{};

		explicit Font(void* font) : _font(font) {}

	public:
		static Font CreateFromFontFile(const std::filesystem::path& fontFilePath, int pointSize);

		/// Copy not allowed
		Font(const Font&) = delete;
		Font& operator=(const Font&) = delete;
		/// Move allowed
		Font(Font&&) noexcept;
		Font& operator=(Font&&) noexcept;
		/// Destructor
		virtual ~Font();

		[[nodiscard]] void* RawHandle() const { return _font; } // TODO remove

		void SetSize(int pointSize);
		void SetWrappedAlign(TextHorizontalAlignment horizontalAlignment);
	};
}
