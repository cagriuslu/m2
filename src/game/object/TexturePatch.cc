#include "m2/game/object/TexturePatch.h"

#include "m2/Game.h"

m2::Id m2::obj::create_texture_patch(const VecF& pos, SDL_Texture* texture, const RectI rect, const int ppm) {
	const auto it = CreateObject(pos);

	auto srcRect = static_cast<SDL_Rect>(rect);

	auto& gfx = it->AddGraphic(BackgroundDrawLayer::B0);
	gfx.onDraw = [=, srcRect = srcRect](const Graphic& gfx_) {
		const auto textureWidthDstPx = RoundI(F(rect.w) * M2_GAME.Dimensions().OutputPixelsPerMeter() / F(ppm));
		const auto textureHeightDstPx = RoundI(F(rect.h) * M2_GAME.Dimensions().OutputPixelsPerMeter() / F(ppm));
		const auto screenOriginToPositionDstPxVec = ScreenOriginToPositionVecPx(gfx_.Owner().position);
		const auto dstRect = SDL_Rect{
		    I(roundf(screenOriginToPositionDstPxVec.x)) - (textureWidthDstPx / 2),
		    I(roundf(screenOriginToPositionDstPxVec.y)) - (textureHeightDstPx / 2),
			textureWidthDstPx,
		    textureHeightDstPx};
		SDL_RenderCopy(M2_GAME.renderer, texture, &srcRect, &dstRect);
	};

	return it.GetId();
}
