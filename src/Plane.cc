#include <m2/m3/Plane.h>
#include <m2/Log.h>

std::pair<m3::VecF,bool> m3::Plane::intersection(const Line& line) const {
	// Plane equation: 0 = Nx(x - Ex) + Ny(y - Ey) + Nz(z - Ez)
	// Line equation: x = Px + Dx.t, y = Py + Dy.t, z = Pz + Dz.t
	// Find t = (Nx(Ex - Px) + Ny(Ey - Py) + Nz(Ez - Pz)) / (NxDx + NyDy + NzDz)
	auto p2e = point - line.point;
	auto nominator = normal.dot(p2e);
	auto denominator = normal.dot(line.direction);
	auto t = nominator / denominator;
	auto x = line.point.x + line.direction.x * t;
	auto y = line.point.y + line.direction.y * t;
	auto z = line.point.z + line.direction.z * t;
	return std::make_pair(VecF{x, y, z}, 0.0f <= t);
}
