#ifndef M2_STRING_H
#define M2_STRING_H

#include "Value.h"
#include <vector>
#include <string>

namespace m2::string {
	std::vector<std::string> split(std::string str, char delimiter);

}

#endif
