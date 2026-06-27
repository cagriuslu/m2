#include <m2/common/math/VecE.h>
#include <m2/common/math/VecF.h>
#include <m2/common/math/VecI.h>

m2::VecE::VecE(const VecF& v) : _x(v.GetX()), _y(v.GetY()) {}

m2::VecE::operator VecF() const {
	return VecF{_x.ToFloat(), _y.ToFloat()};
}
m2::VecE::operator VecI() const {
	return VecI{_x.ToInteger(), _y.ToInteger()};
}

m2::Exact m2::VecE::GetLengthSquaredFE() const {
	return _x * _x + _y * _y;
}
m2::Exact m2::VecE::GetDistanceToSquaredFE(const VecE& other) const {
	return (other - *this).GetLengthSquaredFE();
}

auto std::formatter<m2::VecE>::format(const m2::VecE& vec, std::format_context& ctx) const -> std::format_context::iterator {
	return std::formatter<std::string>::format(
		std::format("{{x:{},y:{}}}", vec.GetX(), vec.GetY()), ctx);
}
