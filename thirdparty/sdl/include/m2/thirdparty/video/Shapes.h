#pragma once
#include <m2/common/math/RectF.h>
#include <m2/common/math/RectI.h>
#include <m2/common/math/VecF.h>
#include <m2/common/video/Color.h>

namespace m2::thirdparty::video {
	class Renderer;

	/// Draws a single filled triangle with a uniform color. The three points are given in logical pixels.
	void FillTriangle(Renderer& renderer, const VecF& point0Lpx, const VecF& point1Lpx, const VecF& point2Lpx, const RGBA& color);
	/// Draws a filled triangle with different colors for each point. The three points are given in logical pixels.
	void FillTriangle(Renderer& renderer, const VecF& point0Lpx, const VecF& point1Lpx, const VecF& point2Lpx, const RGBA& color0, const RGBA& color1, const RGBA& color2);
	/// Draws a filled disk. The center position and radius are given in logical pixels.
	void FillCircle(Renderer& renderer, const VecF& centerLpx, const RGBA& centerColor, float radius, const RGBA& edgeColor, unsigned steps = 96);
	/// Draws a filled rectangle. The position is given in logical pixels.
	void FillRectangle(Renderer& renderer, const RectF& rectLpx, const RGBA& color);

	/// Draws a single point. The position is in logical pixels.
	void DrawPoint(Renderer& renderer, const VecF& pointLpx, const RGBA& color);
	/// Draws a line. The positions are in logical pixels.
	void DrawLine(Renderer& renderer, const VecF& point0Lpx, const VecF& point1Lpx, const RGBA& color);
	/// Draws a rectangle outline. The positions are in logical pixels.
	void DrawRectangle(Renderer& renderer, const RectF& rectLpx, const RGBA& color);
	/// Draws a rectangle outline. The positions are in logical pixels.
	void DrawRectangle(Renderer& renderer, const VecF& centerLpx, float width, float height, float orientationRads, const RGBA& color);
}
