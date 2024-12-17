#include <m2/String.h>
#include <sstream>
#include <utility>

std::vector<std::string> m2::SplitString(std::string str, char delimiter) {
	std::vector<std::string> tokens;
	size_t pos;
	while ((pos = str.find(delimiter)) != std::string::npos) {
		tokens.emplace_back(str.substr(0, pos));
		str.erase(0, pos + 1);
	}
	tokens.emplace_back(str);
	return tokens;
}

std::string m2::TrimRight(std::string str) {
	str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(), str.end());
	return str;
}

std::string m2::TrimLeft(std::string str) {
	str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
		return !std::isspace(ch);
	}));
	return str;
}

std::string m2::Trim(std::string str) {
	return TrimLeft(TrimRight(std::move(str)));
}
