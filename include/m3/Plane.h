#pragma once

#include "Line.h"
#include "VecF.h"
#include <string>

namespace m3 {
	struct Plane {
		VecF normal; // n: <Nx,Nb,Nc>
		VecF point; // e: <Ex, Ey, Ez>
		// Equation: 0 = Nx(x - Ex) + Ny(y - Ey) + Nz(z - Ez)

		inline Plane() = default;
		inline Plane(const VecF& normal, const VecF& point) : normal(normal), point(point) {}
		inline static Plane xy_plane(float z_component = 0.0f) { return {VecF{0.0f, 0.0f, 1.0f}, VecF{0.0f, 0.0f, z_component}}; }

		// True: collision happens in the forwards direction of line
		// False: collision happens in the opposite direction of line
		[[nodiscard]] std::pair<VecF, bool> intersection(const Line& line) const;
	};
}
