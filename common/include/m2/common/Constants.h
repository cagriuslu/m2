#pragma once
#include <cstdint>

namespace m2 {
	constexpr float PI = 3.141592653589793f;
	constexpr float PI_MUL2 = 6.283185307179586f;
	constexpr float PI_MUL3_DIV2 = 4.71238898038469f;
	constexpr float PI_DIV2 = 1.570796326794897f;
	constexpr float SQROOT_2 = 1.414213562373095f;

	constexpr uint32_t DEFAULT_AUDIO_SAMPLE_RATE = 48000;

	enum class MouseButton {
		PRIMARY,
		SECONDARY,
		MIDDLE,
		end
	};

	enum class TextHorizontalAlignment {
		CENTER = 0,
		LEFT,
		RIGHT
	};

	enum class TextVerticalAlignment {
		CENTER = 0,
		TOP,
		BOTTOM
	};
}
