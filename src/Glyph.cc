#include <m2/Glyph.h>
#include <m2/Exception.h>
#include <m2/protobuf/Detail.h>
#include <SDL2/SDL_image.h>
#include <string_view>
#include <vector>

// This vector is accessed using GlyphType enum indexes, thus the values should be consecutive
const std::vector<std::string_view> glyph_resources{
	"",
	"resource/red-circle.svg"
};

bool m2::GlyphsSheet::GlyphKey::operator==(const GlyphKey& other) const {
	return type == other.type && w == other.w && h == other.h;
}
size_t m2::GlyphsSheet::GlyphKeyHash::operator()(const GlyphKey &k) const {
	uint64_t packed = static_cast<uint64_t>(k.w) | (static_cast<uint64_t>(k.h) << 16);
	packed |= static_cast<uint64_t>(k.type) << 32;
	return std::hash<uint64_t>{}(packed);
}

m2::GlyphsSheet::GlyphsSheet(SDL_Renderer* renderer) : DynamicSheet(renderer) {
	// Assert that glyph_resources is the correct size
	if (glyph_resources.size() != (size_t) protobuf::enum_value_count<pb::GlyphType>()) {
		throw M2FATAL("Invalid Glyph resource count");
	}
}
std::pair<SDL_Texture*, SDL_Rect> m2::GlyphsSheet::get_glyph(pb::GlyphType type, int w, int h) {
	GlyphKey key{.type = type, .w = w, .h = h};

	// Check if already renderer
	auto it = _glyphs.find(key);
	if (it != _glyphs.end()) {
		return {texture(), it->second};
	} else {
		SDL_RWops *file = SDL_RWFromFile(glyph_resources[protobuf::enum_index(type)].data(), "rb");
		if (!file) {
			throw M2FATAL("Unable to open glyph file: " + std::string(SDL_GetError()));
		}
		auto* svg_surface = IMG_LoadSizedSVG_RW(file, w, h);
		if (!svg_surface) {
			throw M2FATAL("Unable to open glyph file: " + std::string(IMG_GetError()));
		}

		auto [dst_surface, dst_rect] = alloc(w, h);
		if (SDL_BlitSurface(svg_surface, nullptr, dst_surface, &dst_rect)) {
			throw M2FATAL("Unable to blit glyph: " + std::string(SDL_GetError()));
		}
		SDL_FreeSurface(svg_surface);

		_glyphs[key] = dst_rect;
		return {recreate_texture(), dst_rect};
	}
}
