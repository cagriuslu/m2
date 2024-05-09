#include <m2/RectI.h>
#include <m2/RectF.h>
#include <m2/Meta.h>
#include <sstream>

m2::RectI::RectI() : x(), y(), w(), h() {}
m2::RectI::RectI(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}
m2::RectI::RectI(const RectF& r) : x(static_cast<int>(r.x)), y(static_cast<int>(r.y)), w(static_cast<int>(r.w)), h(static_cast<int>(r.h)) {}
m2::RectI::RectI(const SDL_Rect& r) : x(r.x), y(r.y), w(r.w), h(r.h) {}
m2::RectI::RectI(const pb::RectI& r) : x(r.x()), y(r.y()), w(r.w()), h(r.h()) {}
m2::RectI m2::RectI::from_corners(const m2::VecI &corner1, const m2::VecI &corner2) {
	auto top_left_x = std::min(corner1.x, corner2.x);
	auto top_left_y = std::min(corner1.y, corner2.y);
	auto bottom_right_x = std::max(corner1.x, corner2.x);
	auto bottom_right_y = std::max(corner1.y, corner2.y);
	return {top_left_x, top_left_y, bottom_right_x - top_left_x + 1, bottom_right_y - top_left_y + 1};
}
m2::RectI m2::RectI::from_intersecting_cells(const RectF& rect) {
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
bool m2::RectI::contains(const VecI& p) const {
	return (p.x >= x) && (p.x < (x + w)) && (p.y >= y) && (p.y < (y + h));
}

void m2::RectI::for_each_cell(const std::function<void(const VecI&)>& op) const {
	for (auto j = y; j < y + h; ++j) {
		for (auto i = x; i < x + w; ++i) {
			op({i, j});
		}
	}
}

void m2::RectI::for_difference(const RectI& new_rect, const std::function<void(const VecI&)>& on_addition, const std::function<void(const VecI&)>& on_removal) const {
	auto intersection = intersect(new_rect);
	if (not intersection) {
		// Unload all
		for_each_cell(on_removal);
		// Load all new
		new_rect.for_each_cell(on_addition);
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
	if (new_rect.y2() < y2()) {
		for (auto j = new_rect.y2(); j < y2(); ++j) {
			for (auto i = x; i < x + w; ++i) {
				on_removal({i, j});
			}
		}
	}
	// Unload 3rd rectangle
	if (x < new_rect.x) {
		auto y_start = std::max(y, new_rect.y);
		auto y_limit = std::min(y2(), new_rect.y2());
		for (auto j = y_start; j < y_limit; ++j) {
			for (auto i = x; i < new_rect.x; ++i) {
				on_removal({i, j});
			}
		}
	}
	// Unload 4th rectangle
	if (new_rect.x2() < x2()) {
		auto y_start = std::max(y, new_rect.y);
		auto y_limit = std::min(y2(), new_rect.y2());
		for (auto j = y_start; j < y_limit; ++j) {
			for (auto i = new_rect.x2(); i < x2(); ++i) {
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
			for (auto i = new_rect.x; i < new_rect.x2(); ++i) {
				on_addition({i, j});
			}
		}
	}
	// Load 2nd rectangle
	if (y2() < new_rect.y2()) {
		for (auto j = y2(); j < new_rect.y2(); ++j) {
			for (auto i = new_rect.x; i < new_rect.x2(); ++i) {
				on_addition({i, j});
			}
		}
	}
	// Load 3rd rectangle
	if (new_rect.x < x) {
		auto y_start = std::max(y, new_rect.y);
		auto y_limit = std::min(y2(), new_rect.y2());
		for (auto j = y_start; j < y_limit; ++j) {
			for (auto i = new_rect.x; i < x; ++i) {
				on_addition({i, j});
			}
		}
	}
	// Load 4th rectangle
	if (x2() < new_rect.x2()) {
		auto y_start = std::max(y, new_rect.y);
		auto y_limit = std::min(y2(), new_rect.y2());
		for (auto j = y_start; j < y_limit; ++j) {
			for (auto i = x2(); i < new_rect.x2(); ++i) {
				on_addition({i, j});
			}
		}
	}
}

m2::RectI m2::RectI::trim(int amount) const {
	return RectI{x + amount, y + amount, w - amount - amount, h - amount - amount};
}
m2::RectI m2::RectI::trim_left(int amount) const {
	return (amount < w) ? RectI{x + amount, y, w - amount, h} : RectI{x + w, y, 0, h};
}
m2::RectI m2::RectI::trim_right(int amount) const {
	return (amount < w) ? RectI{x, y, w - amount, h} : RectI{x, y, 0, h};
}
m2::RectI m2::RectI::trim_top(int amount) const {
	return (amount < h) ? RectI{x, y + amount, w, h - amount} : RectI{x, y + h, w, 0};
}
m2::RectI m2::RectI::trim_bottom(int amount) const {
	return (amount < h) ? RectI{x, y, w, h - amount} : RectI{x, y, w, 0};
}
m2::RectI m2::RectI::trim_to_square() const {
	if (h < w) {
		auto diff = w - h;
		return trim_left(diff / 2).trim_right(diff / 2);
	} else if (w < h) {
		auto diff = h - w;
		return trim_top(diff / 2).trim_bottom(diff / 2);
	}
	return *this;
}
m2::RectI m2::RectI::expand(int amount) const {
	return trim(-amount);
}
m2::RectI m2::RectI::horizontal_split(int split_count, int piece_idx) const {
    return RectI{x, y + h * piece_idx / split_count, w, h / split_count};
}
std::optional<m2::RectI> m2::RectI::intersect(const m2::RectI& other) const {
	auto a = static_cast<SDL_Rect>(*this);
	auto b = static_cast<SDL_Rect>(other);
	SDL_Rect result;
	if (SDL_IntersectRect(&a, &b, &result)) {
		return RectI{result};
	} else {
		return {};
	}
}
m2::RectI m2::RectI::ratio(const RectF& ratio_rect) const {
	return RectI{
		I(roundf(F(x) + F(w) * ratio_rect.x)),
		I(roundf(F(y) + F(h) * ratio_rect.y)),
	    I(roundf(F(w) * ratio_rect.w)),
	    I(roundf(F(h) * ratio_rect.h))
	};
}

std::string m2::to_string(const m2::RectI& v) {
	std::stringstream ss;
	ss << "{x:" << v.x << ",y:" << v.y << ",w:" << v.w << ",h:" << v.h << "}";
	return ss.str();
}
