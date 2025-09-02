#include <m2/math/RectI.h>
#include <m2/math/RectF.h>
#include <m2/Meta.h>
#include <sstream>

m2::RectI::RectI() : x(), y(), w(), h() {}
m2::RectI::RectI(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}
m2::RectI::RectI(const RectF& r) : x(static_cast<int>(r.x)), y(static_cast<int>(r.y)), w(static_cast<int>(r.w)), h(static_cast<int>(r.h)) {}
m2::RectI::RectI(const SDL_Rect& r) : x(r.x), y(r.y), w(r.w), h(r.h) {}
m2::RectI::RectI(const pb::RectI& r) : x(r.x()), y(r.y()), w(r.w()), h(r.h()) {}
m2::RectI m2::RectI::CreateFromCorners(const m2::VecI &corner1, const m2::VecI &corner2) {
	auto top_left_x = std::min(corner1.x, corner2.x);
	auto top_left_y = std::min(corner1.y, corner2.y);
	auto bottom_right_x = std::max(corner1.x, corner2.x);
	auto bottom_right_y = std::max(corner1.y, corner2.y);
	return {top_left_x, top_left_y, bottom_right_x - top_left_x + 1, bottom_right_y - top_left_y + 1};
}
m2::RectI m2::RectI::CreateFromIntersectingCells(const RectF& rect) {
	auto x = I(floorf(rect.x));
	auto y = I(floorf(rect.y));
	return {x, y, I(ceilf(rect.x + rect.w)) - x, I(ceilf(rect.y + rect.h)) - y};
}

bool m2::RectI::operator==(const RectI &other) const {
	return (x == other.x) && (y == other.y) && (w == other.w) && (h == other.h);
}
m2::RectI::operator bool() const {
	return 0 < w && 0 < h;
}
m2::RectI::operator SDL_Rect() const {
	return SDL_Rect{x, y, w, h};
}
m2::RectI::operator SDL_FRect() const {
	return SDL_FRect{static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h)};
}
bool m2::RectI::DoesContain(const VecI& p) const {
	return (p.x >= x) && (p.x < (x + w)) && (p.y >= y) && (p.y < (y + h));
}

void m2::RectI::ForEachCell(const std::function<void(const VecI&)>& op) const {
	for (auto j = y; j < y + h; ++j) {
		for (auto i = x; i < x + w; ++i) {
			op({i, j});
		}
	}
}

void m2::RectI::ForDifference(const RectI& new_rect, const std::function<void(const VecI&)>& on_addition, const std::function<void(const VecI&)>& on_removal) const {
	auto intersection = GetIntersection(new_rect);
	if (not intersection) {
		// Unload all
		ForEachCell(on_removal);
		// Load all new
		new_rect.ForEachCell(on_addition);
		return;
	}

	// ***********
	// *    1    *
	// *---***---*
	// * 3 *n* 4 *
	// *---***---*
	// *    2    *
	// ***********

	// Unload 1st rectangle
	if (y < new_rect.y) {
		for (auto j = y; j < new_rect.y; ++j) {
			for (auto i = x; i < x + w; ++i) {
				on_removal({i, j});
			}
		}
	}
	// Unload 2nd rectangle
	if (new_rect.GetY2() < GetY2()) {
		for (auto j = new_rect.GetY2(); j < GetY2(); ++j) {
			for (auto i = x; i < x + w; ++i) {
				on_removal({i, j});
			}
		}
	}
	// Unload 3rd rectangle
	if (x < new_rect.x) {
		auto y_start = std::max(y, new_rect.y);
		auto y_limit = std::min(GetY2(), new_rect.GetY2());
		for (auto j = y_start; j < y_limit; ++j) {
			for (auto i = x; i < new_rect.x; ++i) {
				on_removal({i, j});
			}
		}
	}
	// Unload 4th rectangle
	if (new_rect.GetX2() < GetX2()) {
		auto y_start = std::max(y, new_rect.y);
		auto y_limit = std::min(GetY2(), new_rect.GetY2());
		for (auto j = y_start; j < y_limit; ++j) {
			for (auto i = new_rect.GetX2(); i < GetX2(); ++i) {
				on_removal({i, j});
			}
		}
	}

	// ***********
	// *n   1    *
	// *---***---*
	// * 3 * * 4 *
	// *---***---*
	// *    2    *
	// ***********

	// Load 1st rectangle
	if (new_rect.y < y) {
		for (auto j = new_rect.y; j < y; ++j) {
			for (auto i = new_rect.x; i < new_rect.GetX2(); ++i) {
				on_addition({i, j});
			}
		}
	}
	// Load 2nd rectangle
	if (GetY2() < new_rect.GetY2()) {
		for (auto j = GetY2(); j < new_rect.GetY2(); ++j) {
			for (auto i = new_rect.x; i < new_rect.GetX2(); ++i) {
				on_addition({i, j});
			}
		}
	}
	// Load 3rd rectangle
	if (new_rect.x < x) {
		auto y_start = std::max(y, new_rect.y);
		auto y_limit = std::min(GetY2(), new_rect.GetY2());
		for (auto j = y_start; j < y_limit; ++j) {
			for (auto i = new_rect.x; i < x; ++i) {
				on_addition({i, j});
			}
		}
	}
	// Load 4th rectangle
	if (GetX2() < new_rect.GetX2()) {
		auto y_start = std::max(y, new_rect.y);
		auto y_limit = std::min(GetY2(), new_rect.GetY2());
		for (auto j = y_start; j < y_limit; ++j) {
			for (auto i = GetX2(); i < new_rect.GetX2(); ++i) {
				on_addition({i, j});
			}
		}
	}
}

m2::RectI m2::RectI::TrimAllSides(int amount) const {
	return RectI{x + amount, y + amount, w - amount - amount, h - amount - amount};
}
m2::RectI m2::RectI::TrimLeft(int amount) const {
	return (amount < w) ? RectI{x + amount, y, w - amount, h} : RectI{x + w, y, 0, h};
}
m2::RectI m2::RectI::TrimRight(int amount) const {
	return (amount < w) ? RectI{x, y, w - amount, h} : RectI{x, y, 0, h};
}
m2::RectI m2::RectI::TrimTop(int amount) const {
	return (amount < h) ? RectI{x, y + amount, w, h - amount} : RectI{x, y + h, w, 0};
}
m2::RectI m2::RectI::TrimBottom(int amount) const {
	return (amount < h) ? RectI{x, y, w, h - amount} : RectI{x, y, w, 0};
}
m2::RectI m2::RectI::TrimToSquare() const {
	if (h < w) {
		auto diff = w - h;
		return TrimLeft(diff / 2).TrimRight(diff / 2);
	} else if (w < h) {
		auto diff = h - w;
		return TrimTop(diff / 2).TrimBottom(diff / 2);
	}
	return *this;
}
m2::RectI m2::RectI::TrimToAspectRatio(int desired_w, int desired_h) const {
	auto desired_aspect_ratio = ToFloat(desired_w) / ToFloat(desired_h);
	auto current_aspect_ratio = ToFloat(w) / ToFloat(h);
	if (desired_aspect_ratio == current_aspect_ratio) {
		return *this;
	}

	// If desired aspect ratio is wider than current aspect ratio
	if (current_aspect_ratio < desired_aspect_ratio) {
		// Trim top and bottom
		auto desired_height = RoundI(ToFloat(w) / desired_aspect_ratio);
		auto height_diff = h - desired_height;
		return this->TrimTop(height_diff / 2).TrimBottom(height_diff / 2);
	} else {
		// If desired aspect ratio is longer than current aspect ratio, trim left and right
		auto desired_width = RoundI(ToFloat(h) * desired_aspect_ratio);
		auto width_diff = w - desired_width;
		return this->TrimLeft(width_diff / 2).TrimRight(width_diff / 2);
	}
}
m2::RectI m2::RectI::AlignLeftTo(int _x) const {
	return {_x, y, w, h};
}
m2::RectI m2::RectI::AlignRightTo(int _x) const {
	return {_x - w, y, w, h};
}
m2::RectI m2::RectI::AlignTopTo(int _y) const {
	return {x, _y, w, h};
}
m2::RectI m2::RectI::AlignBottomTo(int _y) const {
	return {x, _y - h, w, h};
}
m2::RectI m2::RectI::AlignCenterTo(int _x, int _y) const {
	return {_x - w/2, _y - h/2, w, h};
}
m2::RectI m2::RectI::ExpandAllSides(int amount) const {
	return TrimAllSides(-amount);
}
m2::RectI m2::RectI::GetRow(const int totalRowCount, const int rowIndex) const {
    return RectI{x, y + h * rowIndex / totalRowCount, w, h / totalRowCount};
}
std::optional<m2::RectI> m2::RectI::GetIntersection(const m2::RectI& other) const {
	auto a = static_cast<SDL_Rect>(*this);
	auto b = static_cast<SDL_Rect>(other);
	SDL_Rect result;
	if (SDL_IntersectRect(&a, &b, &result) == SDL_TRUE) {
		return RectI{result};
	} else {
		return std::nullopt;
	}
}
m2::RectI m2::RectI::ApplyRatio(const RectF& ratio_rect) const {
	return RectI{
		I(roundf(ToFloat(x) + ToFloat(w) * ratio_rect.x)),
		I(roundf(ToFloat(y) + ToFloat(h) * ratio_rect.y)),
	    I(roundf(ToFloat(w) * ratio_rect.w)),
	    I(roundf(ToFloat(h) * ratio_rect.h))
	};
}

std::string m2::ToString(const m2::RectI& v) {
	std::stringstream ss;
	ss << "{x:" << v.x << ",y:" << v.y << ",w:" << v.w << ",h:" << v.h << "}";
	return ss.str();
}
