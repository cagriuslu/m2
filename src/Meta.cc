#include <m2/Meta.h>
#include <cstdio>

int m2::RoundDownToEvenI(const float f) {
	const auto ceiled = iceil(f);
	return ceiled % 2 // If even
			? ceiled - 1 // Subtract one
			: ceiled;
}

std::string m2::ToString(float n, unsigned precision) {
	auto format = std::string("%.0") + m2::ToString(precision) + "f";
	// Get size of printed string
	int size = std::snprintf(nullptr, 0, format.c_str(), n);
	// Allocate string
	std::string s(size, '\0');
	// Print
	std::snprintf(s.data(), s.length() + 1, format.c_str(), n);
	return s;
}
