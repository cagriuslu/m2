#include <m2/thirdparty/video/Font.h>
#include <m2/common/Error.h>
#include <SDL3_ttf/SDL_ttf.h>

using namespace m2;
using namespace m2::thirdparty::video;

namespace {
	TTF_HorizontalAlignment ToTtfWrapAlignment(TextHorizontalAlignment horizontalAlignment) {
		switch (horizontalAlignment) {
			case TextHorizontalAlignment::LEFT:  return TTF_HORIZONTAL_ALIGN_LEFT;
			case TextHorizontalAlignment::RIGHT: return TTF_HORIZONTAL_ALIGN_RIGHT;
			default:                             return TTF_HORIZONTAL_ALIGN_CENTER;
		}
	}
}

Font Font::CreateFromFontFile(const std::filesystem::path& fontFilePath, const int pointSize) {
	if (auto* font = TTF_OpenFont(fontFilePath.string().c_str(), static_cast<float>(pointSize))) {
		return Font{font};
	}
	throw M2_ERROR(std::string{"Unable to open font: "} + fontFilePath.string() + ", " + SDL_GetError());
}

void Font::SetSize(const float pointSize) {
	TTF_SetFontSize(static_cast<TTF_Font*>(_font), pointSize);
}
void Font::SetWrappedAlign(const TextHorizontalAlignment horizontalAlignment) {
	TTF_SetFontWrapAlignment(static_cast<TTF_Font*>(_font), ToTtfWrapAlignment(horizontalAlignment));
}

Font::Font(Font&& other) noexcept : _font(other._font) { other._font = nullptr; }
Font& Font::operator=(Font&& other) noexcept { std::swap(_font, other._font); return *this; }
Font::~Font() {
	if (_font) {
		TTF_CloseFont(static_cast<TTF_Font*>(_font));
		_font = nullptr;
	}
}
