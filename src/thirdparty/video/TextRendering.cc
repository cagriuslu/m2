#include <m2/thirdparty/video/TextRendering.h>

int m2::thirdparty::video::CalculateMaxRenderedUtf8Length(TTF_Font* font, const int fontSize, const char* utf8Text, const int widthInPixels) {
	TTF_SetFontSize(font, fontSize);
	int extent = 0, croppedGlyphCount = 0;
	TTF_MeasureUTF8(font, utf8Text, widthInPixels, &extent, &croppedGlyphCount);
	return croppedGlyphCount;
}

m2::VecI m2::thirdparty::video::CalculateRenderedUtf8Size(TTF_Font* font, int fontSize, const char* utf8Text) {
	TTF_SetFontSize(font, fontSize);
	int weight = 0, heigth = 0;
	TTF_SizeUTF8(font, utf8Text, &weight, &heigth);
	return VecI{weight, heigth};
}
