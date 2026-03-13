#pragma once
#include <m2/math/VecI.h>
#include <SDL2/SDL_ttf.h>

namespace m2::thirdparty::video {
	/// Calculates how many characters of a UTF8 string would fit into given width. Changes the size of the font,
	/// invalidating the glyph caches. widthInPixels is the width of the space where the text is being attempted to be
	/// drawn. Applicable to drawing only unwrapped text.
	int CalculateMaxRenderedUtf8Length(TTF_Font* font, int fontSize, const char* utf8Text, int widthInPixels);

	/// Calculates how many pixels the text would occupy if rendered at the given font size. Changes the size of the
	/// font, invalidating the glyph caches.
	VecI CalculateRenderedUtf8Size(TTF_Font* font, int fontSize, const char* utf8Text);
}
