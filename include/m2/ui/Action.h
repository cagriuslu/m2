#pragma once

namespace m2::ui {
	enum class Action {
		CONTINUE = 0, // UI keeps running
		GAIN_FOCUS, // UI keeps running
		LOSE_FOCUS, // UI keeps running
		RETURN, // UI returns
		QUIT // Application returns
	};
}
