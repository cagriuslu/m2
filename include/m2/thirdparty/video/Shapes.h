#pragma once
#include <m2/math/VecF.h>
#include <m2/video/Color.h>

namespace m2::thirdparty::video {
	/// Draws a single filled triangle with a uniform color. The three points are given in screen space (pixels).
	void FillTriangle(const VecF& point0, const VecF& point1, const VecF& point2, const RGBA& color);
}
