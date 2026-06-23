#include <m2/video/TextLabel.h>
#include <m2/thirdparty/video/Surface.h>
#include <m2/thirdparty/video/Detail.h>
#include <m2/Game.h>

m2::VecI m2::EstimateTextLabelDimensions(TTF_Font* font, const std::string& text, int fontSize) {
	// Change the font size. This operation clears the glyph caches, but that's a sacrifice I'm willing to make.
	TTF_SetFontSize(font, fontSize);

	// Estimate the size of the final text
	int w, h;
	const auto sizeResult = TTF_SizeUTF8(font, text.c_str(), &w, &h);
	m2ExpectZeroOrThrowMessage(sizeResult, TTF_GetError());
	return {w, h};
}

m2::VecF m2::TextLabelCenterToOriginVectorInSourcePixels(const pb::TextLabel& tl) {
	const auto centerToOriginM = VecF{-tl.push_dimensions().x(), -tl.push_dimensions().y()};
	// Text labels are pre-rendered at the same PPM as the game (source PPM is 1), thus we need to multiply only
	// with the PPM of the game to convert from source pixels to destination pixels.
	return centerToOriginM * M2_GAME.Dimensions().OutputPixelsPerMeter();
}

/// Returns a vector from the screen origin (top-left) to the center of this sprite in output pixel units.
m2::VecF m2::ScreenOriginToTextLabelCenterVecOutpx(const pb::TextLabel& tl, const VecF& position) {
	return ScreenOriginToPositionVecPx(position) - TextLabelCenterToOriginVectorInOutputPixels(tl);
}

int m2::FontSizeOfTextLabel(const pb::TextLabel& tl) {
	return I(tl.height_m() * M2_GAME.Dimensions().OutputPixelsPerMeter());
}

void m2::DrawTextLabelBackgroundIn2dWorld(const pb::TextLabel& tl, const RectI& sourceRect, const VecF& position, const bool isDimmed) {
	const auto backgroundColor = RGBA{tl.background_color()};
	const auto trueBackgroundColor = isDimmed ? backgroundColor * M2G_PROXY.dimming_factor : backgroundColor;
	RectF rect{
		position.GetX() - ToFloat(sourceRect.w) / 2.0f / M2_GAME.Dimensions().OutputPixelsPerMeter(),
		position.GetY() - ToFloat(sourceRect.h) / 2.0f / M2_GAME.Dimensions().OutputPixelsPerMeter(),
		ToFloat(sourceRect.w) / M2_GAME.Dimensions().OutputPixelsPerMeter(),
		ToFloat(sourceRect.h) / M2_GAME.Dimensions().OutputPixelsPerMeter()
	};
	// Push the text label
	rect.x += tl.push_dimensions().x();
	rect.y += tl.push_dimensions().y();
	Graphic::ColorRect(rect, trueBackgroundColor);
}
void m2::DrawTextLabelIn2dWorld(const pb::TextLabel& tl, const RectI& sourceRect, const VecF& position, const float angle, MAYBE bool is_foreground, MAYBE float z) {
	const auto sourceRectSdl = thirdparty::video::ToSdlRect(sourceRect);
	DrawTextureIn2dWorld(
			*M2_GAME.renderer,
			static_cast<SDL_Texture*>(M2_GAME.GetTextLabelCache().Texture().RawHandle()),
			&sourceRectSdl,
			0.0f,
			1.0f,
			TextLabelCenterToOriginVectorInOutputPixels(tl),
			ScreenOriginToTextLabelCenterVecOutpx(tl, position),
			angle);
}
void m2::DrawTextLabelIn3dWorld(const pb::TextLabel& tl, const RectI& sourceRect, const VecF& position, const float angle, const bool is_foreground, const float z) {
	const auto sourceRectSdl = thirdparty::video::ToSdlRect(sourceRect);
	DrawTextureIn3dWorld(
			*M2_GAME.renderer,
			static_cast<SDL_Texture*>(M2_GAME.GetTextLabelCache().Texture().RawHandle()),
			&sourceRectSdl,
			M2_GAME.Dimensions().OutputPixelsPerMeter(),
			TextLabelCenterToOriginVectorInOutputPixels(tl),
			0.0f,
			static_cast<VecF>(M2_GAME.GetTextLabelCache().Texture().Dimensions()),
			position,
			z,
			angle,
			is_foreground);
}
void m2::SlowDrawSystemTextIn2dWorld(const char* str, const VecF& position) {
	auto surface = thirdparty::video::Surface::RenderTextSolid(M2_GAME.systemFont, str, RGBA{255, 255, 255, 255});
	auto texture = thirdparty::video::Texture::CreateFromSurface(*M2_GAME.renderer, surface.RawHandle());
	const auto dimensions = surface.Dimensions();
	const SDL_Rect srcRect{0, 0, dimensions.x, dimensions.y};
	DrawTextureIn2dWorld(*M2_GAME.renderer, static_cast<SDL_Texture*>(texture.RawHandle()), &srcRect, 0.0f, 1.0f, {}, ScreenOriginToPositionVecPx(position), 0.0f);
}

m2::RectI m2::TextLabelCache::TextLabelGenerator::operator()(const std::tuple<std::string,int>& item) {
	// Change the font size. This operation clears the glyph caches, but that's a sacrifice I'm willing to make.
	TTF_SetFontSize(_font, I(item));

	// Estimate the size of the final text
	int w, h;
	const auto sizeResult = TTF_SizeUTF8(_font, std::get<std::string>(item).c_str(), &w, &h);
	m2ExpectZeroOrThrowMessage(sizeResult, TTF_GetError());

	// Render to new surface
	auto renderSurface = thirdparty::video::Surface::RenderTextBlended(_font, std::get<std::string>(item), RGBA{255, 255, 255, 255});

	// Blit new surface to allocated surface
	return *_dynamicSheet.AllocateAndMutate(w, h, [&](SDL_Surface* surface, const RectI& area) {
		auto dstRectSdl = thirdparty::video::ToSdlRect(area);
		const auto blitResult = SDL_BlitSurface(static_cast<SDL_Surface*>(renderSurface.RawHandle()), nullptr, surface, &dstRectSdl);
		m2ExpectZeroOrThrowMessage(blitResult, SDL_GetError());
	}, false);
}

size_t m2::TextLabelCache::TextLabelHash::operator()(const std::tuple<std::string,int>& item) const {
	return std::hash<std::string>{}(std::get<0>(item)) ^ std::hash<int>{}(std::get<1>(item));
}
