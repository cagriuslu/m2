#include <m2/third_party/physics/box2d/DebugDraw.h>
#include <m2/Game.h>
#include <m2/component/Graphic.h>
#include <m2/video/Color.h>
#include <m2/math/VecF.h>

using namespace m2::third_party::physics::box2d;

DebugDraw::DebugDraw() {
	AppendFlags(e_shapeBit);
	AppendFlags(e_centerOfMassBit);
}

void DebugDraw::DrawPolygon(const b2Vec2* vertices, const int32 vertexCount, const b2Color& color) {
	for (int i = 0; i < vertexCount - 1; ++i) {
		Graphic::DrawLine(VecF{vertices[i]}, VecF{vertices[i + 1]}, RGBA{color});
	}
}
void DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, const int32 vertexCount, const b2Color& color) {
	for (int i = 0; i < vertexCount - 1; ++i) {
		Graphic::DrawLine(VecF{vertices[i]}, VecF{vertices[i + 1]}, RGBA{color});
	}
}
void DebugDraw::DrawCircle(const b2Vec2& center, const float radius, const b2Color& color) {
	if (IsProjectionTypeParallel(M2_LEVEL.GetProjectionType())) {
		// Draw a true circle from the cache
		const int r = RoundI(M2_GAME.Dimensions().OutputPixelsPerMeter() * radius);
		const auto srcRect = static_cast<SDL_Rect>(M2_GAME.GetShapeCache().Create(std::make_shared<Circle>(r)));
		auto* texture = M2_GAME.GetShapeCache().Texture();
		const auto screenOriginToSpriteCenter = ScreenOriginToPositionVecPx(VecF{center});
		const auto dstRect = SDL_Rect{
			RoundI(screenOriginToSpriteCenter.x) - srcRect.w / 2,
			RoundI(screenOriginToSpriteCenter.y) - srcRect.h / 2,
			srcRect.w,
			srcRect.h};
		const RGBA colorU8{color};
		SDL_SetTextureColorMod(texture, colorU8.r, colorU8.g, colorU8.b);
		SDL_RenderCopy(M2_GAME.renderer, texture, &srcRect, &dstRect);
	} else {
		const auto centerPosition = m3::VecF{VecF{center}};
		// Draw an octagon instead of circle
		const auto corner = radius / sqrtf(2.0f);
		const auto pointTop = ScreenOriginToProjectionAlongCameraPlaneDstpx(centerPosition.offset_y(-radius));
		const auto pointTopRight = ScreenOriginToProjectionAlongCameraPlaneDstpx(centerPosition.offset_x(corner).offset_y(-corner));
		const auto pointRight = ScreenOriginToProjectionAlongCameraPlaneDstpx(centerPosition.offset_x(radius));
		const auto pointBottomRight = ScreenOriginToProjectionAlongCameraPlaneDstpx(centerPosition.offset_x(corner).offset_y(corner));
		const auto pointBottom = ScreenOriginToProjectionAlongCameraPlaneDstpx(centerPosition.offset_y(radius));
		const auto pointBottomLeft = ScreenOriginToProjectionAlongCameraPlaneDstpx(centerPosition.offset_x(-corner).offset_y(corner));
		const auto pointLeft = ScreenOriginToProjectionAlongCameraPlaneDstpx(centerPosition.offset_x(-radius));
		const auto pointTopLeft = ScreenOriginToProjectionAlongCameraPlaneDstpx(centerPosition.offset_x(-corner).offset_y(-corner));
		if (pointTop && pointTopRight && pointRight && pointBottomRight && pointBottom && pointBottomLeft && pointLeft && pointTopLeft) {
			const RGBA colorU8{color};
			SDL_SetRenderDrawColor(M2_GAME.renderer, colorU8.r, colorU8.g, colorU8.b, colorU8.a);
			const std::array points = {
				SDL_FPoint{pointTop->x, pointTop->y},
				SDL_FPoint{pointTopRight->x, pointTopRight->y},
				SDL_FPoint{pointRight->x, pointRight->y},
				SDL_FPoint{pointBottomRight->x, pointBottomRight->y},
				SDL_FPoint{pointBottom->x, pointBottom->y},
				SDL_FPoint{pointBottomLeft->x, pointBottomLeft->y},
				SDL_FPoint{pointLeft->x, pointLeft->y},
				SDL_FPoint{pointTopLeft->x, pointTopLeft->y},
				SDL_FPoint{pointTop->x, pointTop->y}
			};
			SDL_RenderDrawLinesF(M2_GAME.renderer, points.data(), I(points.size()));
		}
	}
}
void DebugDraw::DrawSolidCircle(const b2Vec2& center, const float radius, MAYBE const b2Vec2& axis, const b2Color& color) {
	DrawCircle(center, radius, color);
}
void DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, MAYBE const b2Color& color) {
	// Segments of a chain
	Graphic::DrawLine(VecF{p1}, VecF{p2}, RGBA{255, 0, 0, 255});
}
void DebugDraw::DrawTransform(const b2Transform& xf) {
	// Origin of an object
	Graphic::DrawCross(VecF{xf.p}, 0.25f, {255, 255, 0, 255});
}
void DebugDraw::DrawPoint(const b2Vec2& p, const float size, const b2Color& color) {
	// Points of an edge
	Graphic::DrawCross(VecF{p}, size, RGBA{color});
}
