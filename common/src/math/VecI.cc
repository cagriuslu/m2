#include <m2/common/Meta.h>
#include <m2/common/math/VecF.h>
#include <m2/common/math/VecI.h>
#include <m2/common/math/VecE.h>

m2::VecI::VecI(const VecF& v) : VecI(v.GetX(), v.GetY()) {}
m2::VecI::VecI(const VecE& v) : x(v.GetX().Round().ToInteger()), y(v.GetY().Round().ToInteger()) {}

auto std::formatter<m2::VecI>::format(const m2::VecI& vec, std::format_context& ctx) const -> std::format_context::iterator {
	return std::formatter<std::string>::format(
		std::format("{{x:{},y:{}}}", vec.x, vec.y), ctx);
}
