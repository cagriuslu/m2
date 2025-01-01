#include <m2/Meta.h>
#include <cstdio>

int m2::RoundDownToEvenI(const float f) {
	const auto ceiled = iceil(f);
	return ceiled % 2 // If even
			? ceiled - 1 // Subtract one
			: ceiled;
}

std::string m2::to_string(bool b) {
	return b ? "true" : "false";
}
std::string m2::to_string(int n) {
	return std::to_string(n);
}
std::string m2::to_string(unsigned int n) {
	return std::to_string(n);
}
std::string m2::to_string(long n) {
	return std::to_string(n);
}
std::string m2::to_string(unsigned long n) {
	return std::to_string(n);
}
std::string m2::to_string(long long n) {
	return std::to_string(n);
}
std::string m2::to_string(unsigned long long n) {
	return std::to_string(n);
}
std::string m2::to_string(float n) {
	return std::to_string(n);
}
std::string m2::to_string(float n, unsigned precision) {
	auto format = std::string("%.0") + std::to_string(precision) + "f";
	// Get size of printed string
	int size = std::snprintf(nullptr, 0, format.c_str(), n);
	// Allocate string
	std::string s(size, '\0');
	// Print
	std::snprintf(s.data(), s.length() + 1, format.c_str(), n);
	return s;
}
std::string m2::to_string(double n) {
	return std::to_string(n);
}
std::string m2::to_string(const char* s) {
	return {s};
}
std::string m2::to_string(const std::string& s) {
	return s;
}
std::string m2::to_string(std::string_view sv) {
	return std::string{sv};
}
