#include <m2/M2.h>

const std::string m2::empty_string;

std::string m2::ToString(BackgroundLayer layer) {
	return ToString(I(layer));
}

size_t m2::utf8_codepoint_count(const char* s) {
	size_t len = 0;
	while (*s) {
		len += ((*s & 0xc0) != 0x80) ? 1 : 0;
		++s;
	}
	return len;
}
