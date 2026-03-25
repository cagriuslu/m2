#include <m2/math/RectE.h>
#include <m2/math/RectF.h>

m2::RectE::operator RectF() const {
	return RectF{x.ToFloat(), y.ToFloat(), w.ToFloat(), h.ToFloat()};
}
