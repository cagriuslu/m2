#pragma once
#include <m2/video/DynamicSheet.h>
#include <m2/common/containers/Cache.h>
#include <m2/common/math/VecF.h>
#include <m2/thirdparty/video/Font.h>
#include <Sprite.pb.h>

namespace m2 {
	// Utilities for drawing text labels

	/// Returns a vector from the text label's center to the text label's origin in source pixels.
	VecF TextLabelCenterToOriginVectorInSourcePixels(const pb::TextLabel&);

	/// Returns a vector from the text label's center to the text label's origin in logical pixel units.
	inline VecF TextLabelCenterToOriginVectorInLogicalPixels(const pb::TextLabel& tl) {
		return TextLabelCenterToOriginVectorInSourcePixels(tl);
	}

	/// Returns a vector from the screen origin (top-left) to the center of the text label in logical pixel units.
	VecF ScreenOriginToTextLabelCenterVecLpx(const pb::TextLabel& tl, const VecF& position);

	int FontSizeOfTextLabel(const pb::TextLabel&);

	void DrawTextLabelBackgroundIn2dWorld(const pb::TextLabel& tl, const RectI& sourceRect, const VecF& position, bool isDimmed);
	void DrawTextLabelIn2dWorld(const pb::TextLabel&, const RectI& sourceRect, const VecF& position, float angle, bool is_foreground = {}, float z = {});
	void DrawTextLabelIn3dWorld(const pb::TextLabel&, const RectI& sourceRect, const VecF& position, float angle, bool is_foreground = {}, float z = {});
	void SlowDrawSystemTextIn2dWorld(const char* str, const VecF& position);

	/// Currently, this cache can only store unwrapped text.
	class TextLabelCache {

		/// Generator used in Cache
		class TextLabelGenerator {
			DynamicSheet _dynamicSheet;
			thirdparty::video::Font& _font;
		public:
			explicit TextLabelGenerator(thirdparty::video::Renderer& renderer, thirdparty::video::Font& font) : _dynamicSheet(renderer), _font(font) {}
			[[nodiscard]] const thirdparty::video::Texture& Texture() const { return _dynamicSheet.Texture(); }
			RectI operator()(const std::tuple<std::string,float>& item);
		};
		/// Hash function used in Cache
		struct TextLabelHash {
			size_t operator()(const std::tuple<std::string,float>& item) const;
		};

		Cache<
			std::tuple<std::string,float>, // Key
			RectI, // Value
			TextLabelGenerator, // Value generator
			TextLabelHash // Key hash function
		> _cache;

	public:
		TextLabelCache(thirdparty::video::Renderer& renderer, thirdparty::video::Font& font) : _cache(TextLabelGenerator{renderer, font}) {}

		// Accessors

		[[nodiscard]] const thirdparty::video::Texture& Texture() const { return _cache.Generator().Texture(); }

		// Modifiers

		RectI Create(const std::string& text, const float fontSize) { return _cache(std::make_tuple(text, fontSize)); }
	};
}
