#pragma once
#include <m2/math/VecF.h>

namespace m2 {
	class Line {
		/// Parallel is a vector that is passing through the origin and parallel to this line.
		/// Length of the parallel isn't significant, but the direction is.
		VecF _parallel{};
		/// Any point on the line
		VecF _point{};

		Line(const VecF parallel, const VecF point) : _parallel(parallel), _point(point) {}

	public:
		static Line FromParallelAndPoint(const VecF parallel, const VecF point) { return Line{parallel, point}; }
		static Line FromPoints(const VecF& p1, const VecF& p2) { return FromParallelAndPoint(p2 - p1, p1); }

		// Accessors

		[[nodiscard]] const VecF& GetParallel() const { return _parallel; }
		[[nodiscard]] VecF GetPerpendicular() const { return {-_parallel.GetY(), _parallel.GetX()}; }
		[[nodiscard]] float GetSlope() const { return _parallel.GetY() / _parallel.GetX(); }
		[[nodiscard]] float GetXIntersect() const;
		[[nodiscard]] float GetYIntersect() const;
		[[nodiscard]] std::optional<VecF> GetIntersectionPointWith(const Line& other) const;
		/// Rotation needed for the parallel of this line to align with the other line. [-PI,+PI]
		[[nodiscard]] float GetAngleTo(const Line& other) const;
		/// Rotation needed for this line to align with the other line. [-PI_DIV2, PI_DIV2]
		[[nodiscard]] float GetSmallerAngleTo(const Line& other) const;

		// Immutable Modifiers

		[[nodiscard]] Line GetPerpendicularAtPoint(const VecF& point) const { return FromParallelAndPoint(GetPerpendicular(), point); }
	};
}
