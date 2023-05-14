#include <m2/Meta.h>

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

std::string m2::to_string(double n) {
	return std::to_string(n);
}

std::string m2::to_string(const char* s) {
	return {s};
}

std::string m2::to_string(const std::string& s) {
	return s;
}
