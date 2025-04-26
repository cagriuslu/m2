#pragma once
#include "Meta.h"

#define MAYBE [[maybe_unused]] // TODO get rid of this
#define IF(cond) if (cond) cond // TODO make this a function
#define IF_TYPE(ref, type) if (std::holds_alternative<type>(ref)) std::get<type>(ref) // TODO make this a function

namespace m2 {
	extern const std::string gEmptyString;

	/// Background layers are drawn from back to front. B0 is drawn the last, thus it's on the front. The only
	/// difference between the background layers is the drawing order. The rigid bodies of background objects are still
	/// placed on the same physics world as F0.
	enum class BackgroundLayer {
		B0 = 0,
		B1 = 1,
		B2 = 2,
		B3 = 3,
		_n // End sentinel
	};
	constexpr int gBackgroundLayerCount = static_cast<int>(BackgroundLayer::_n);
	std::string ToString(BackgroundLayer layer);

	/// Foreground layers are drawn from back to front. L0 is drawn the first, thus it's on the back. Each foreground
	/// layer designates a distinct physics world, thus the objects in different layers cannot collide with each other.
	/// Since all background objects place their bodies into F0, objects in other layers cannot collide with them.
	enum class ForegroundLayer {
		F0 = 0,
		F1 = 1,
		_n // End sentinel
	};
	constexpr int gForegroundLayerCount = static_cast<int>(ForegroundLayer::_n);
	std::string ToString(ForegroundLayer layer);

	/// Returns the number of codepoints in a UTF-8 string.
	/// One codepoint does not always equate to one glyph, but it's a good estimation for most characters.
	size_t Utf8CodepointCount(const char* s);

	/// This function is called by main to store the initial stack position.
	void StoreInitialStackPosition();
	/// This function can be called to query the current stack position (the difference between initial and current stack pointers).
	size_t GetCurrentStackPosition();
}
