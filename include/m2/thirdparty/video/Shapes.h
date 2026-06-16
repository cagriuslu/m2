#pragma once
#include <m2/math/RectF.h>
#include <m2/math/RectI.h>
#include <m2/math/VecF.h>
#include <m2/video/Color.h>

namespace m2::thirdparty::video {
	/// Draws a single filled triangle with a uniform color. The three points are given in screen space (pixels).
	void FillTriangle(const VecF& point0, const VecF& point1, const VecF& point2, const RGBA& color);
	/// Draws a filled disk. The center position and radius are given in screen space (pixels).
	void FillCircle(const VecF& centerPx, const RGBA& centerColor, float radiusPx, const RGBA& edgeColor, unsigned steps = 96);
	/// Draws a filled rectangle. The position is given in screen space (pixels).
	void FillRectangle(const RectF& rectPx, const RGBA& color);
	/// Draws a filled rectangle. The position is given in screen space (pixels).
	void FillRectangle(const RectI& rectPx, const RGBA& color);
}
