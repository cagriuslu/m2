#pragma once

#include "VecF.h"

namespace m3 {
	struct Line {
		VecF direction; // d: <Dx,Dy,Dz>
		VecF point; // p: <Px,Py,Pz>
		// Equation 1: (x - Px) / Dx = (y - Py) / Dy = (z - Pz) / Dz = t
		// Equation 2: x = Px + Dx.t, y = Py + Dy.t, z = Pz + Dz.t

		inline Line() = default;
		inline Line(const VecF& direction, const VecF& point) : direction(direction), point(point) {}
		inline static Line from_points(const VecF& from, const VecF& to) { return Line{to - from, from}; }
	};
}
