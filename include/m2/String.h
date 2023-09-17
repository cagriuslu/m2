#pragma once
#include "Meta.h"
#include <vector>
#include <string>

namespace m2::string {
	std::vector<std::string> split(std::string str, char delimiter);
	std::string trim_right(std::string str);
	std::string trim_left(std::string str);
	std::string trim(std::string str);
}
