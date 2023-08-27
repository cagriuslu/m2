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

		explicit operator bool() const;
		explicit operator SDL_FRect() const;
		explicit operator SDL_Rect() const;

		[[nodiscard]] float area() const;
		[[nodiscard]] RectF shift_origin(const VecF& direction) const;
		[[nodiscard]] std::optional<RectF> intersect(const RectF& other) const;
		[[nodiscard]] std::vector<VecI> intersecting_cells() const;
	};
}
