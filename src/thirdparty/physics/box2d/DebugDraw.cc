#include <m2/thirdparty/physics/box2d/DebugDraw.h>
#include <m2/Game.h>
#include <m2/component/Graphic.h>
#include <m2/video/Color.h>
#include <m2/thirdparty/physics/box2d/Detail.h>
#include <m2/common/math/VecF.h>
#include <array>

using namespace m2::thirdparty::physics::box2d;

DebugDraw::DebugDraw() {
	AppendFlags(e_shapeBit);
	AppendFlags(e_centerOfMassBit);
}

void DebugDraw::DrawPolygon(const b2Vec2* vertices, const int32 vertexCount, const b2Color& color) {
	for (int i = 0; i < vertexCount - 1; ++i) {
		Graphic::DrawLine(ToVecF(vertices[i]), ToVecF(vertices[i + 1]), RGBA{color});
	}
}
void DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, const int32 vertexCount, const b2Color& color) {
	for (int i = 0; i < vertexCount - 1; ++i) {
		Graphic::DrawLine(ToVecF(vertices[i]), ToVecF(vertices[i + 1]), RGBA{color});
	}
}
void DebugDraw::DrawCircle(const b2Vec2& center, const float radius, const b2Color& color) {
	if (IsProjectionTypeParallel(M2_LEVEL.GetProjectionType())) {
		// Draw a true circle from the cache
		const int r = RoundI(M2_GAME.Dimensions().OutputPixelsPerMeter() * radius);
		const RectI srcRect = M2_GAME.GetShapeCache().Create(std::make_shared<Circle>(r));
		const auto& texture = M2_GAME.GetShapeCache().Texture();
		const auto screenOriginToSpriteCenter = ScreenOriginToPositionVecPx(ToVecF(center));
		const RectI dstRect{
			RoundI(screenOriginToSpriteCenter.GetX()) - srcRect.w / 2,
			RoundI(screenOriginToSpriteCenter.GetY()) - srcRect.h / 2,
			srcRect.w,
			srcRect.h};
		const auto colorModGuard = texture.ScopedColorMod(static_cast<RGB>(RGBA{color}));
		texture.Render(*M2_GAME.renderer, srcRect, dstRect);
	} else {
		const auto centerPosition = m3::VecF{ToVecF(center)};
		// Draw an octagon instead of circle
		const auto corner = radius / sqrtf(2.0f);
		const auto pointTop         = ScreenOriginToProjectionAlongCameraPlaneDstpx(centerPosition.offset_y(-radius));
		const auto pointTopRight    = ScreenOriginToProjectionAlongCameraPlaneDstpx(centerPosition.offset_x(corner).offset_y(-corner));
		const auto pointRight       = ScreenOriginToProjectionAlongCameraPlaneDstpx(centerPosition.offset_x(radius));
		const auto pointBottomRight = ScreenOriginToProjectionAlongCameraPlaneDstpx(centerPosition.offset_x(corner).offset_y(corner));
		const auto pointBottom      = ScreenOriginToProjectionAlongCameraPlaneDstpx(centerPosition.offset_y(radius));
		const auto pointBottomLeft  = ScreenOriginToProjectionAlongCameraPlaneDstpx(centerPosition.offset_x(-corner).offset_y(corner));
		const auto pointLeft        = ScreenOriginToProjectionAlongCameraPlaneDstpx(centerPosition.offset_x(-radius));
		const auto pointTopLeft     = ScreenOriginToProjectionAlongCameraPlaneDstpx(centerPosition.offset_x(-corner).offset_y(-corner));
		if (pointTop && pointTopRight && pointRight && pointBottomRight && pointBottom && pointBottomLeft && pointLeft && pointTopLeft) {
			const std::array points = {
				*pointTop, *pointTopRight, *pointRight, *pointBottomRight,
				*pointBottom, *pointBottomLeft, *pointLeft, *pointTopLeft, *pointTop};
			M2_GAME.renderer->DrawLineStrip(points, RGBA{color});
		}
	}
}
void DebugDraw::DrawSolidCircle(const b2Vec2& center, const float radius, MAYBE const b2Vec2& axis, const b2Color& color) {
	DrawCircle(center, radius, color);
}
void DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, MAYBE const b2Color& color) {
	// Segments of a chain
	Graphic::DrawLine(ToVecF(p1), ToVecF(p2), RGBA{255, 0, 0, 255});
}
void DebugDraw::DrawTransform(const b2Transform& xf) {
	// Origin of an object
	Graphic::DrawCross(ToVecF(xf.p), 0.25f, {255, 255, 0, 255});
}
void DebugDraw::DrawPoint(const b2Vec2& p, const float size, const b2Color& color) {
	// Points of an edge
	Graphic::DrawCross(ToVecF(p), size, RGBA{color});
}
