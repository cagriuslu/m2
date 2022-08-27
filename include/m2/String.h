#ifndef M2_STRING_H
#define M2_STRING_H

#include "Value.h"
#include <vector>
#include <string>

namespace m2::string {
	std::vector<std::string> split(std::string str, char delimiter);
	Value<std::string> read_file(const std::string& path);
	VoidValue write_to_file(const std::string& str, const std::string& path);
}

#endif
