#pragma once
#include "Meta.h"

#define MAYBE [[maybe_unused]] // TODO get rid of this
#define IF(cond) if (cond) cond // TODO make this a function
#define IF_TYPE(ref, type) if (std::holds_alternative<type>(ref)) std::get<type>(ref) // TODO make this a function

namespace m2 {
	extern const std::string empty_string;

	enum class BackgroundLayer {
		L0 = 0,
		L1 = 1,
		L2 = 2,
		L3 = 3,
		n
	};
	std::string ToString(BackgroundLayer layer);

	/// Returns the number of codepoints in a UTF-8 string.
	/// One codepoint does not always equate to one glyph, but it's a good estimation for most characters.
	size_t utf8_codepoint_count(const char* s);
}
