#include <m2/thirdparty/video/TextRendering.h>
#include <SDL2/SDL_ttf.h>

int m2::thirdparty::video::CalculateMaxRenderedUtf8Length(Font& font, const int fontSize, const char* utf8Text, const int widthInPixels) {
	font.SetSize(fontSize);
	int extent = 0, croppedGlyphCount = 0;
	TTF_MeasureUTF8(static_cast<TTF_Font*>(font.RawHandle()), utf8Text, widthInPixels, &extent, &croppedGlyphCount);
	return croppedGlyphCount;
}

m2::VecI m2::thirdparty::video::CalculateRenderedUtf8Size(Font& font, int fontSize, const char* utf8Text) {
	font.SetSize(fontSize);
	int weight = 0, heigth = 0;
	TTF_SizeUTF8(static_cast<TTF_Font*>(font.RawHandle()), utf8Text, &weight, &heigth);
	return VecI{weight, heigth};
}
