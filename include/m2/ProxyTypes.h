#pragma once
#include <m2/math/primitives/Fixed.h>
#include <m2/math/primitives/Float.h>
#include <m2/BuildOptions.h>

namespace m2 {
	using FF = std::conditional_t<GAME_IS_DETERMINISTIC, Fixed, Float>;
}