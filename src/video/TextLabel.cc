#include <m2/video/TextLabel.h>
#include <m2/thirdparty/video/Surface.h>
#include <m2/thirdparty/video/TextRendering.h>
#include <m2/Game.h>

m2::VecF m2::TextLabelCenterToOriginVectorInSourcePixels(const pb::TextLabel& tl) {
	const auto centerToOriginM = VecF{-tl.push_dimensions().x(), -tl.push_dimensions().y()};
	// Despite the name, this returns a vector in logical pixels: it's derived from meters via the game's logical
	// PPM, independent of the glyph's physical texel count.
	// TODO fix the name
	return centerToOriginM * M2_GAME.Dimensions().LogicalPixelsPerMeter();
}

/// Returns a vector from the screen origin (top-left) to the center of this sprite in logical pixel units.
m2::VecF m2::ScreenOriginToTextLabelCenterVecLpx(const pb::TextLabel& tl, const VecF& position) {
	return ScreenOriginToPositionVecLpx(position) - TextLabelCenterToOriginVectorInLogicalPixels(tl);
}

int m2::FontSizeOfTextLabel(const pb::TextLabel& tl) {
	const auto dpiX = M2_GAME.GetRenderer().GetPixelsPerWindowUnit().GetX();
	return I(tl.height_m() * M2_GAME.Dimensions().LogicalPixelsPerMeter() * dpiX);
}

void m2::DrawTextLabelBackgroundIn2dWorld(const pb::TextLabel& tl, const RectI& sourceRect, const VecF& position, const bool isDimmed) {
	const auto backgroundColor = RGBA{tl.background_color()};
	const auto trueBackgroundColor = isDimmed ? backgroundColor * M2G_PROXY.dimming_factor : backgroundColor;
	const auto dpiX = M2_GAME.GetRenderer().GetPixelsPerWindowUnit().GetX();
	const auto srcPxPerMeter = M2_GAME.Dimensions().LogicalPixelsPerMeter() * dpiX;
	RectF rect{
		position.GetX() - ToFloat(sourceRect.w) / 2.0f / srcPxPerMeter,
		position.GetY() - ToFloat(sourceRect.h) / 2.0f / srcPxPerMeter,
		ToFloat(sourceRect.w) / srcPxPerMeter,
		ToFloat(sourceRect.h) / srcPxPerMeter
	};
	// Push the text label
	rect.x += tl.push_dimensions().x();
	rect.y += tl.push_dimensions().y();
	Graphic::ColorRect(rect, trueBackgroundColor);
}
void m2::DrawTextLabelIn2dWorld(const pb::TextLabel& tl, const RectI& sourceRect, const VecF& position, const float angle, MAYBE bool is_foreground, MAYBE float z) {
	const auto dpiX = M2_GAME.GetRenderer().GetPixelsPerWindowUnit().GetX();
	DrawTextureIn2dWorld(
			M2_GAME.GetTextLabelCache().Texture(),
			sourceRect,
			0.0f,
			1.0f / dpiX,
			TextLabelCenterToOriginVectorInLogicalPixels(tl),
			ScreenOriginToTextLabelCenterVecLpx(tl, position),
			angle);
}
void m2::DrawTextLabelIn3dWorld(const pb::TextLabel& tl, const RectI& sourceRect, const VecF& position, const float angle, const bool is_foreground, const float z) {
	const auto dpiX = M2_GAME.GetRenderer().GetPixelsPerWindowUnit().GetX();
	DrawTextureIn3dWorld(
			M2_GAME.GetTextLabelCache().Texture(),
			sourceRect,
			M2_GAME.Dimensions().LogicalPixelsPerMeter() * dpiX,
			TextLabelCenterToOriginVectorInLogicalPixels(tl),
			0.0f,
			static_cast<VecF>(M2_GAME.GetTextLabelCache().Texture().Dimensions()),
			position,
			z,
			angle,
			is_foreground);
}
void m2::SlowDrawSystemTextIn2dWorld(const char* str, const VecF& position) {
	auto surface = thirdparty::video::Surface::RenderTextSolid(M2_GAME.systemFont, str, RGBA{255, 255, 255, 255});
	auto texture = thirdparty::video::Texture::CreateFromSurface(M2_GAME.GetRenderer(), surface.RawHandle());
	const auto dimensions = surface.Dimensions();
	const RectI srcRect{0, 0, dimensions.x, dimensions.y};
	DrawTextureIn2dWorld(texture, srcRect, 0.0f, 1.0f, {}, ScreenOriginToPositionVecLpx(position), 0.0f);
}

m2::RectI m2::TextLabelCache::TextLabelGenerator::operator()(const std::tuple<std::string,float>& item) {
	// Sets the font size as a side effect, then renders at that size
	const auto renderedSize = thirdparty::video::CalculateRenderedUtf8Size(_font, std::get<float>(item), std::get<std::string>(item).c_str());

	// Render to new surface
	auto renderSurface = thirdparty::video::Surface::RenderTextBlended(_font, std::get<std::string>(item), RGBA{255, 255, 255, 255});

	// Blit new surface to allocated surface
	return *_dynamicSheet.AllocateAndMutate(CeilI(renderedSize.GetX()), CeilI(renderedSize.GetY()), [&](thirdparty::video::Surface& surface, const RectI& area) {
		m2SucceedOrThrowError(surface.Blit(renderSurface, std::nullopt, area));
	}, false);
}

size_t m2::TextLabelCache::TextLabelHash::operator()(const std::tuple<std::string,float>& item) const {
	return std::hash<std::string>{}(std::get<0>(item)) ^ std::hash<float>{}(std::get<1>(item));
}
