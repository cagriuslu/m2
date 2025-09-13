#pragma once
#include <m2/math/primitives/Exact.h>
#include <m2/math/primitives/Float.h>
#include <m2/BuildOptions.h>

namespace m2 {
	using FE = std::conditional_t<GAME_IS_DETERMINISTIC, Exact, Float>;
}