#pragma once
#include <SDL2/SDL_rect.h>
#include <RectI.pb.h>
#include "../math/VecF.h"
#include <optional>

namespace m2 {
	struct RectI;

	struct RectF {
		float x, y, w, h;

		RectF();
		RectF(float x, float y, float w, float h);
		RectF(const VecF& xy, float w, float h);
		explicit RectF(const RectI& r);
		explicit RectF(const SDL_FRect& r);
		explicit RectF(const SDL_Rect& r);
		explicit RectF(const pb::RectI& r);
		static RectF CreateCenteredAround(const VecF& center, float w, float h) { return {center.GetX() - w / 2.0f, center.GetY() - h / 2.0f, w, h}; }
		static RectF CreateCenteredAround(const VecF& center, const VecF& dims) { return CreateCenteredAround(center, dims.GetX(), dims.GetY()); }
		static RectF CreateFromCorners(const VecF& corner1, const VecF& corner2);

		// Operator

		explicit operator bool() const;
		explicit operator SDL_FRect() const;
		explicit operator SDL_Rect() const;

		// Accessors

		[[nodiscard]] float GetX2() const { return x + w; }
		[[nodiscard]] float GetY2() const { return y + h; }
		[[nodiscard]] VecF GetTopLeftPoint() const { return VecF{x, y}; }
		[[nodiscard]] VecF GetTopRightPoint() const { return VecF{x + w, y}; }
		[[nodiscard]] VecF GetBottomLeftPoint() const { return VecF{x, y + h}; }
		[[nodiscard]] VecF GetBottomRightPoint() const { return VecF{x + w, y + h}; }
		[[nodiscard]] float GetDiagonalLength() const { return sqrtf(w * w + h * h); }
		[[nodiscard]] float GetArea() const;
		[[nodiscard]] bool IsEqual(const RectF& other, float tolerance = 0.001f) const;
		[[nodiscard]] bool DoesContain(const VecF&, float tolerance = 0.001f) const;
		[[nodiscard]] bool DoesContain(const RectF&, float tolerance = 0.001f) const;

		// Immutable modifiers

		[[nodiscard]] RectF Shift(const VecF& direction) const; // Shifts the rect
		[[nodiscard]] RectF ShiftCoordinateSystemOrigin(const VecF& direction) const; // Shifts the origin of the coordinate system
		[[nodiscard]] RectF ExpandAllSides(float amount) const;
		[[nodiscard]] std::optional<RectF> GetIntersection(const RectF& other, float tolerance = 0.001f) const;
		[[nodiscard]] std::vector<VecI> GetIntersectingCells() const;
		[[nodiscard]] VecF GetCenterPoint() const;
	};

	std::string ToString(const RectF&);
}
