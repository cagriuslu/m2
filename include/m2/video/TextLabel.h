#pragma once
#include <m2/video/DynamicSheet.h>
#include <m2/containers/Cache.h>
#include <m2/math/VecF.h>
#include <SDL2/SDL_ttf.h>
#include <Sprite.pb.h>

namespace m2 {
	// Utilities for drawing text labels

	VecI EstimateTextLabelDimensions(TTF_Font* font, const std::string& text, int fontSize);

	/// Returns a vector from the text label's center to the text label's origin in source pixels.
	VecF TextLabelCenterToOriginVectorInSourcePixels(const pb::TextLabel&);

	/// Returns a vector from the text label's center to the text label's origin in output pixel units.
	/// For text labels, this value is the same as source pixels, because text labels are rendered 1-1.
	inline VecF TextLabelCenterToOriginVectorInOutputPixels(const pb::TextLabel& tl) {
		return TextLabelCenterToOriginVectorInSourcePixels(tl);
	}

	/// Returns a vector from the screen origin (top-left) to the center of the text label in output pixel units.
	VecF ScreenOriginToTextLabelCenterVecOutpx(const pb::TextLabel& tl, const VecF& position);

	int FontSizeOfTextLabel(const pb::TextLabel&);

	void DrawTextLabelBackgroundIn2dWorld(const pb::TextLabel& tl, const RectI& sourceRect, const VecF& position, bool isDimmed);
	void DrawTextLabelBackgroundIn3dWorld(const pb::TextLabel& tl, const RectI& sourceRect, const VecF& position, bool isDimmed);
	void DrawTextLabelIn2dWorld(const pb::TextLabel&, const RectI& sourceRect, const VecF& position, float angle, bool is_foreground = {}, float z = {});
	void DrawTextLabelIn3dWorld(const pb::TextLabel&, const RectI& sourceRect, const VecF& position, float angle, bool is_foreground = {}, float z = {});

	/// Currently, this cache can only store unwrapped text.
	class TextLabelCache {

		/// Generator used in Cache
		class TextLabelGenerator {
			DynamicSheet _dynamicSheet;
			TTF_Font* _font;
		public:
			explicit TextLabelGenerator(SDL_Renderer* renderer, TTF_Font* font) : _dynamicSheet(renderer, false), _font(font) {}
			[[nodiscard]] SDL_Texture* Texture() const { return _dynamicSheet.Texture(); }
			RectI operator()(const std::tuple<std::string,int>& item);
		};
		/// Hash function used in Cache
		struct TextLabelHash {
			size_t operator()(const std::tuple<std::string,int>& item) const;
		};

		Cache<
				std::tuple<std::string,int>, // Key
				RectI, // Value
				TextLabelGenerator, // Value generator
				TextLabelHash // Key hash function
				> _cache;

	public:
		TextLabelCache(SDL_Renderer* renderer, TTF_Font* font) : _cache(TextLabelGenerator{renderer, font}) {}

		// Accessors

		[[nodiscard]] SDL_Texture* Texture() const { return _cache.Generator().Texture(); }

		// Modifiers

		RectI Create(const std::string& text, const int fontSize) { return _cache(std::make_tuple(text, fontSize)); }
	};
}
