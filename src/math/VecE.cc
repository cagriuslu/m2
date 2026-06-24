#include <m2/common/math/VecE.h>
#include <m2/math/VecF.h>
#include <m2/math/VecI.h>

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

std::string m2::ToString(const VecE& v) {
	std::stringstream ss;
	ss << "{x:" << ToString(v.GetX()) << ",y:" << ToString(v.GetY()) << "}";
	return ss.str();
}
