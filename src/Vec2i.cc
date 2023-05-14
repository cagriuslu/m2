#include <m2/Vec2i.h>
#include <m2/Vec2f.h>
#include <unordered_map>
#include <sstream>

m2::Vec2i::Vec2i(const m2::Vec2f& v) : Vec2i(v.x, v.y) {}

void m2::Vec2i::for_each_cell_in_between(const Vec2i& other, const std::function<void(const Vec2i&)>& f) const {
	auto to_right = (x <= other.x);
	auto to_down = (y <= other.y);
	for (int32_t yi = y; to_down ? yi <= other.y : other.y <= yi; to_down ? ++yi : --yi) {
		for (int32_t xi = x; to_right ? xi <= other.x : other.x <= xi; to_right ? ++xi : --xi) {
			f({xi, yi});
		}
	}
}

std::string m2::to_string(const m2::Vec2i& v) {
	std::stringstream ss;
	ss << "{x:" << v.x << ",y:" << v.y << "}";
	return ss.str();
}
