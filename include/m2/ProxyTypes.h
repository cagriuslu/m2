#pragma once
#include <m2/BuildOptions.h>

namespace m2 {
	class Exact;
	class Float;
	class VecE;
	class VecF;

	using FE = std::conditional_t<GAME_IS_DETERMINISTIC, Exact, Float>;
	using VecFE = std::conditional_t<GAME_IS_DETERMINISTIC, VecE, VecF>;
}