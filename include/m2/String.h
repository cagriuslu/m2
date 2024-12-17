#pragma once
#include "Meta.h"
#include <vector>
#include <string>

namespace m2 {
	std::vector<std::string> SplitString(std::string str, char delimiter);
	std::string TrimRight(std::string str);
	std::string TrimLeft(std::string str);
	std::string Trim(std::string str);
}
