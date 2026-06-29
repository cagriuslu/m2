#include <m2/common/math/RectE.h>
#include <m2/common/math/RectF.h>

m2::RectE::operator m2::RectF() const {
	return RectF{x.ToFloat(), y.ToFloat(), w.ToFloat(), h.ToFloat()};
}
