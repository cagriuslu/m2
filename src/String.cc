#include <m2/string.hh>

std::vector<std::string> m2::string::split(std::string str, char delimiter) {
	std::vector<std::string> tokens;
	size_t pos;
	while ((pos = str.find(delimiter)) != std::string::npos) {
		tokens.emplace_back(str.substr(0, pos));
		str.erase(0, pos + 1);
	}
	tokens.emplace_back(str);
	return tokens;
}
