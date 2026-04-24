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
}
