#include <m2/math/VecFF.h>
#include <m2/math/VecF.h>
#include <m2/math/VecI.h>

m2::VecFF::VecFF(const VecI& v) : _x(v.x), _y(v.y) {}
m2::VecFF::VecFF(const pb::VecI& v) : _x(v.x()), _y(v.y()) {}
m2::VecFF::VecFF(const pb::VecFF& v) :
	_x(decltype(_x)::FromProtobufRepresentation(v.x().value_e6())),
	_y(decltype(_y)::FromProtobufRepresentation(v.y().value_e6())) {}

m2::VecFF::operator VecF() const {
	return {_x.ToFloat(), _y.ToFloat()};
}
