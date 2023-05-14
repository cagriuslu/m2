#include <m2/Vec2i.h>
#include <m2/Vec2f.h>
#include <m2/Vson.h>
#include <m2/Meta.h>
#include <unordered_map>

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

namespace {
	m2::Vson to_vson(const m2::Vec2i& v) {
		auto vs = m2::Vson::object();
		vs["x"] = m2::to_string(v.x);
		vs["y"] = m2::to_string(v.y);
		return vs;
	}
}

std::string m2::to_string(const m2::Vec2i& v) {
	return to_vson(v).dump_to_string();
}
std::string m2::to_string(const std::vector<Vec2i>& vec) {
	auto vs = Vson::array();
	for (const auto& v : vec) {
		vs.push_back(to_vson(v));
	}
	return vs.dump_to_string();
}
