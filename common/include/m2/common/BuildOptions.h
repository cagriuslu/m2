#pragma once

namespace m2 {
#ifdef DEBUG
	constexpr bool BUILD_IS_DEBUG = true;
#else
	constexpr bool BUILD_IS_DEBUG = false;
#endif

#ifdef _GAME_IS_DETERMINISTIC
	constexpr bool GAME_IS_DETERMINISTIC = true;
#else
	constexpr bool GAME_IS_DETERMINISTIC = false;
#endif

	class Exact;
	class Float;
	using FE = std::conditional_t<GAME_IS_DETERMINISTIC, Exact, Float>;
	class VecE;
	class VecF;
	using VecFE = std::conditional_t<GAME_IS_DETERMINISTIC, VecE, VecF>;

#ifdef _USE_M2_PHYSICS
	constexpr bool USE_M2_PHYSICS = true;
#else
	constexpr bool USE_M2_PHYSICS = false;
#endif
}
