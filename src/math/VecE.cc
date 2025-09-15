#include <m2/math/VecE.h>
#include <m2/math/VecF.h>

m2::VecE::VecE(const VecF& v) : _x(v.GetX()), _y(v.GetY()) {}

m2::VecE::operator VecF() const {
	return VecF{_x.ToFloat(), _y.ToFloat()};
}
