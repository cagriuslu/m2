#pragma once
#include <optional>
#include <string>

namespace m2 {
	std::vector<std::string> to_argument_list(int argc, char** argv);
	std::optional<std::string> parse_argument(const std::vector<std::string>& argument_list, const std::string& argument_key);
}  // namespace m2
