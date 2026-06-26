#include <m2/thirdparty/video/TextRendering.h>
#include <SDL3_ttf/SDL_ttf.h>

int m2::thirdparty::video::CalculateMaxRenderedUtf8Length(Font& font, const float fontSize, const char* utf8Text, const int widthInPixels) {
	font.SetSize(fontSize);
	int measuredWidthPx = 0;
	size_t measuredLengthBytes = 0;
	// SDL3_ttf reports the byte length of the prefix that fits within widthInPixels (SDL2 reported a glyph
	// count). For the single-byte text this is used with, byte length and glyph count coincide.
	TTF_MeasureString(static_cast<TTF_Font*>(font.RawHandle()), utf8Text, /*length=*/0, widthInPixels, &measuredWidthPx, &measuredLengthBytes);
	return static_cast<int>(measuredLengthBytes);
}

m2::VecF m2::thirdparty::video::CalculateRenderedUtf8Size(Font& font, const float fontSize, const char* utf8Text) {
	font.SetSize(fontSize);
	int weight = 0, heigth = 0;
	TTF_GetStringSize(static_cast<TTF_Font*>(font.RawHandle()), utf8Text, /*length=*/0, &weight, &heigth);
	return VecF{weight, heigth};
}
