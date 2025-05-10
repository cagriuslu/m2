#pragma once
#include <m2/math/VecF.h>

namespace m2 {
	class Line {
		/// Parallel is a vector that is passing through the origin and parallel to this line.
		/// Lenght of the parallel isn't significant.
		VecF _parallel{};
		/// Any point on the line
		VecF _point{};

		Line(const VecF parallel, const VecF point) : _parallel(parallel), _point(point) {}

	public:
		static Line FromParallelAndPoint(const VecF parallel, const VecF point) { return Line{parallel, point}; }
		static Line FromPoints(const VecF& p1, const VecF& p2) { return FromParallelAndPoint(p2 - p1, p1); }

		// Accessors

		[[nodiscard]] const VecF& Parallel() const { return _parallel; }
		[[nodiscard]] VecF PerpendicularParallel() const { return {-_parallel.y, _parallel.x}; }
		[[nodiscard]] float Slope() const { return _parallel.y / _parallel.x; }
		[[nodiscard]] float XIntersect() const;
		[[nodiscard]] float YIntersect() const;
		[[nodiscard]] std::optional<VecF> IntersectionWith(const Line& other) const;
		/// Angle between positive X-axis and this line. (-PI_DIV2,PI_DIV2]
		[[nodiscard]] float Angle() const;
		/// Rotation needed to for this line to align with other line. [0,PI)
		[[nodiscard]] float AngleTo(const Line& other) const;
		/// Smaller of the two angles between this and other line. [0, PI_DIV2)
		[[nodiscard]] float SmallerAngleBetween(const Line& other) const;

		// Immutable Modifiers

		[[nodiscard]] Line PerpendicularAtPoint(const VecF& point) const { return FromParallelAndPoint(PerpendicularParallel(), point); }
	};
}
