#pragma once

namespace m2 {
#ifdef _GAME_IS_DETERMINISTIC
	constexpr bool GAME_IS_DETERMINISTIC = true;
#else
	constexpr bool GAME_IS_DETERMINISTIC = false;
#endif
}
