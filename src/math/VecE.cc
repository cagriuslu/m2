#include <m2/math/VecE.h>
#include <m2/math/VecF.h>

m2::VecE::VecE(const VecF& v) : _x(v.GetX()), _y(v.GetY()) {}

m2::VecE::operator VecF() const {
	return VecF{_x.ToFloat(), _y.ToFloat()};
}

m2::Exact m2::VecE::GetLengthSquaredFE() const {
	return _x * _x + _y * _y;
}
m2::Exact m2::VecE::GetDistanceToSquaredFE(const VecE& other) const {
	return (other - *this).GetLengthSquaredFE();
}
