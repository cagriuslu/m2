#include <m2/Meta.h>
#include <m2/math/Rational.h>
#include <m2/math/VecF.h>
#include <m2/math/VecI.h>
#include <m2/Vson.h>

#include <unordered_map>

m2::VecI::VecI(const m2::VecF& v) : VecI(v.x, v.y) {}

m2::VecI m2::VecI::GetDimensionsInAspectRatio(int w, int h) const {
	// Simplify the w_ to h_ ratio
	auto aspect_ratio = Rational{w, h}.Simplify();
	auto simplified_w = aspect_ratio.GetN();
	auto simplified_h = aspect_ratio.GetD();

	// Do the math one order above
	auto dims = *this;
	auto dims_x1000 = dims * 1000;

	// Assume the height is an integer multiple of the aspect ratio,
	// find the multiplier that takes the dimension from aspect ratio to full dimensions.
	auto multiplier_x1000 = dims_x1000.y / simplified_h;
	// What would the width be if the height was truly an integer multiple of the aspect ratio
	auto presumed_width_x1000 = simplified_w * multiplier_x1000;

	int64_t correct_width, correct_height;
	if (presumed_width_x1000 < dims_x1000.x) {
		// Full dimensions are wider than the presumed width, we need to squeeze from the sides
		correct_width = presumed_width_x1000 / 1000;  // Presumed width is already squeezed
		correct_height = dims.y;
	} else {
		// Full dimensions are less wide than the presumed width,
		// we need to assume the width as an integer multiple of the aspect ratio.
		// Find the new multiplier that takes the dimension from aspect ratio to full dimensions
		multiplier_x1000 = dims_x1000.x / simplified_w;
		// What would the height be in this case
		auto presumed_height_x1000 = simplified_h * multiplier_x1000;

		correct_width = dims.x;
		correct_height = presumed_height_x1000 / 1000;  // Presumed height is already squeezed
	}

	return VecI{I(correct_width), I(correct_height)};
}

namespace {
	m2::Vson to_vson(const m2::VecI& v) {
		auto vs = m2::Vson::object();
		vs["x"] = m2::ToString(v.x);
		vs["y"] = m2::ToString(v.y);
		return vs;
	}
}  // namespace

std::string m2::ToString(const m2::VecI& v) { return to_vson(v).dump_to_string(); }
std::string m2::ToString(const std::vector<VecI>& vec) {
	auto vs = Vson::array();
	for (const auto& v : vec) {
		vs.push_back(to_vson(v));
	}
	return vs.dump_to_string();
}
