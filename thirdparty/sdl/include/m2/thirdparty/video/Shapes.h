#pragma once
#include <m2/common/math/RectF.h>
#include <m2/common/math/RectI.h>
#include <m2/common/math/VecF.h>
#include <m2/common/video/Color.h>

namespace m2::thirdparty::video {
	class Renderer;

	/// Draws a single filled triangle with a uniform color. The three points are given in window coordinates.
	void FillTriangle(Renderer& renderer, const VecF& point0, const VecF& point1, const VecF& point2, const RGBA& color);
	/// Draws a filled triangle with different colors for each point. The three points are given in window coordinates.
	void FillTriangle(Renderer& renderer, const VecF& point0, const VecF& point1, const VecF& point2, const RGBA& color0, const RGBA& color1, const RGBA& color2);
	/// Draws a filled disk. The center position and radius are given in window coordinates.
	void FillCircle(Renderer& renderer, const VecF& center, const RGBA& centerColor, float radius, const RGBA& edgeColor, unsigned steps = 96);
	/// Draws a filled rectangle. The position is given in window coordinates.
	void FillRectangle(Renderer& renderer, const RectF& rect, const RGBA& color);

	/// Draws a single point. The position is in window coordinates.
	void DrawPoint(Renderer& renderer, const VecF& point, const RGBA& color);
	/// Draws a line. The positions are in window coordinates.
	void DrawLine(Renderer& renderer, const VecF& point0, const VecF& point1, const RGBA& color);
	/// Draws a rectangle outline. The positions are in window coordinates.
	void DrawRectangle(Renderer& renderer, const RectF& rect, const RGBA& color);
	/// Draws a rectangle outline. The positions are in window coordinates.
	void DrawRectangle(Renderer& renderer, const VecF& center, float width, float height, float orientationRads, const RGBA& color);
}
