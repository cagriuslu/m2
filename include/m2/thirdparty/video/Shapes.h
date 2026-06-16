#pragma once
#include <m2/math/VecF.h>
#include <m2/video/Color.h>

namespace m2::thirdparty::video {
	/// Draws a single filled triangle with a uniform color. The three points are given in screen space (pixels).
	void FillTriangle(const VecF& point0, const VecF& point1, const VecF& point2, const RGBA& color);
	/// Draws a filled disk. The center position and radius are given in screen space (pixels).
	void FillCircle(const VecF& centerPx, const RGBA& centerColor, float radiusPx, const RGBA& edgeColor, unsigned steps = 96);
}
