#pragma once

#include <m2/math/VecF.h>
#include <m2/math/VecI.h>

namespace m2 {
	VecF tile_position_f(const VecI& v);
	VecI tile_position_i(const VecF& v);
}
