#pragma once
#include <SDL2/SDL_rect.h>
#include <RectI.pb.h>
#include "VecF.h"
#include <optional>

namespace m2 {
	struct RectI;

	struct RectF {
		float x, y, w, h;

		RectF();
		RectF(float x, float y, float w, float h);
		RectF(const VecF& offset_from_origin, const VecF& dims);
		explicit RectF(const RectI& r);
		explicit RectF(const SDL_FRect& r);
		explicit RectF(const SDL_Rect& r);
		explicit RectF(const pb::RectI& r);
		static inline RectF centered_around(const VecF& center, float w, float h) { return {center.x - w / 2.0f, center.y - h / 2.0f, w, h}; }
		static RectF from_corners(const VecF& corner1, const VecF& corner2);

		explicit operator bool() const;
		explicit operator SDL_FRect() const;
		explicit operator SDL_Rect() const;
		[[nodiscard]] inline VecF top_left() const { return VecF{x, y}; }
		[[nodiscard]] inline VecF top_right() const { return VecF{x + w, y}; }
		[[nodiscard]] inline VecF bottom_left() const { return VecF{x, y + h}; }
		[[nodiscard]] inline VecF bottom_right() const { return VecF{x + w, y + h}; }

		[[nodiscard]] float area() const;
		[[nodiscard]] RectF shift(const VecF& direction) const; // Shifts the rect
		[[nodiscard]] RectF shift_origin(const VecF& direction) const; // Shifts the origin of the coordinate system
		[[nodiscard]] RectF expand(float amount) const;
		[[nodiscard]] std::optional<RectF> intersect(const RectF& other) const;
		[[nodiscard]] std::vector<VecI> intersecting_cells() const;
		[[nodiscard]] VecF center() const;
	};
}
