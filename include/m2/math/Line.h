#pragma once
#include <m2/math/VecF.h>

namespace m2 {
	class Line {
		/// Parallel is a vector that is passing through the origin and parallel to this line.
		/// Lenght of the parallel isn't significant.
		VecF _parallel{};
		/// Any point on the line
		VecF _point{};

		Line(VecF parallel, VecF point) : _parallel(std::move(parallel)), _point(std::move(point)) {}

	public:
		static Line FromParallelAndPoint(VecF parallel, VecF point) { return Line{std::move(parallel), std::move(point)}; }
		static Line FromPoints(const VecF& p1, const VecF& p2) { return FromParallelAndPoint(p2 - p1, p1); }

		// Accessors

		[[nodiscard]] const VecF& Parallel() const { return _parallel; }
		[[nodiscard]] VecF PerpendicularParallel() const { return {-_parallel.y, _parallel.x}; }
		[[nodiscard]] float Slope() const { return _parallel.y / _parallel.x; }

		// Immutable Modifiers

		[[nodiscard]] Line PerpendicularAtPoint(const VecF& point) const { return FromParallelAndPoint(PerpendicularParallel(), point); }
	};
}
