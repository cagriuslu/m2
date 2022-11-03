#ifndef M2_GLYPH_H
#define M2_GLYPH_H

#include "DynamicSheet.h"
#include <Glyph.pb.h>
#include <string_view>
#include <array>

namespace m2 {
	class GlyphsSheet : private DynamicSheet {
		struct GlyphKey {
			pb::GlyphType type;
			unsigned w, h;

			bool operator==(const GlyphKey& other) const;
		};
		struct GlyphKeyHash {
			size_t operator()(const GlyphKey& k) const;
		};

		std::unordered_map<GlyphKey, SDL_Rect, GlyphKeyHash> _glyphs;
	public:
		explicit GlyphsSheet(SDL_Renderer* renderer);
		std::pair<SDL_Texture*, SDL_Rect> get_glyph(pb::GlyphType type, unsigned w, unsigned h);
	};
}

#endif //M2_GLYPH_H
