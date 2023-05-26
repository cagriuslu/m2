#include <m2/VecI.h>
#include <m2/VecF.h>
#include <m2/Vson.h>
#include <m2/Meta.h>
#include <unordered_map>

m2::VecI::VecI(const m2::VecF& v) : VecI(v.x, v.y) {}

void m2::VecI::for_each_cell_in_between(const VecI& other, const std::function<void(const VecI&)>& f) const {
	auto to_right = (x <= other.x);
	auto to_down = (y <= other.y);
	for (int32_t yi = y; to_down ? yi <= other.y : other.y <= yi; to_down ? ++yi : --yi) {
		for (int32_t xi = x; to_right ? xi <= other.x : other.x <= xi; to_right ? ++xi : --xi) {
			f({xi, yi});
		}
	}
}

namespace {
	m2::Vson to_vson(const m2::VecI& v) {
		auto vs = m2::Vson::object();
		vs["x"] = m2::to_string(v.x);
		vs["y"] = m2::to_string(v.y);
		return vs;
	}
}

std::string m2::to_string(const m2::VecI& v) {
	return to_vson(v).dump_to_string();
}
std::string m2::to_string(const std::vector<VecI>& vec) {
	auto vs = Vson::array();
	for (const auto& v : vec) {
		vs.push_back(to_vson(v));
	}
	return vs.dump_to_string();
}
