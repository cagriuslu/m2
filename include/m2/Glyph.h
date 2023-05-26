#pragma once
#include "DynamicSheet.h"
#include <Glyph.pb.h>

namespace m2 {
	/// Sheet that generated and holds Glyphs on demand
	class GlyphsSheet : private DynamicSheet {
		struct GlyphKey {
			pb::GlyphType type;
			int w, h;

			bool operator==(const GlyphKey& other) const;
		};
		struct GlyphKeyHash {
			size_t operator()(const GlyphKey& k) const;
		};

		std::unordered_map<GlyphKey, SDL_Rect, GlyphKeyHash> _glyphs;

	public:
		explicit GlyphsSheet(SDL_Renderer* renderer);
		std::pair<SDL_Texture*, SDL_Rect> get_glyph(pb::GlyphType type, int w, int h);
	};
}
