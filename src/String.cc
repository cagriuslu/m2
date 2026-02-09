#include <m2/String.h>
#include <sstream>
#include <utility>

std::vector<std::string> m2::SplitString(std::string str, const char delimiter) {
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
std::pair<std::string::const_iterator, std::string::const_iterator> m2::GetTrimmedRange(const std::string& s) {
	const auto start = std::ranges::find_if_not(s, [](const char c) { return std::isspace(c); });
	const auto rend = std::find_if_not(s.rbegin(), s.rend(), [](const char c) { return std::isspace(c); });
	const auto end = rend.base();
	if (end <= start) {
		return std::make_pair(s.end(), s.end());
	}
	return std::make_pair(start, end);
}
std::pair<std::string_view::const_iterator, std::string_view::const_iterator> m2::GetTrimmedRange(std::string_view s) {
	const auto start = std::ranges::find_if_not(s, [](const char c) { return std::isspace(c); });
	const auto rend = std::find_if_not(s.rbegin(), s.rend(), [](const char c) { return std::isspace(c); });
	const auto end = rend.base();
	if (end <= start) {
		return std::make_pair(s.end(), s.end());
	}
	return std::make_pair(start, end);
}
std::string_view m2::GetTrimmedView(const std::string& s) {
	const auto range = GetTrimmedRange(s);
	if (range.first == s.end()) {
		return std::string_view{};
	}
	return std::string_view{range.first, range.second};
}
std::string_view m2::GetTrimmedView(std::string_view s) {
	const auto range = GetTrimmedRange(s);
	if (range.first == s.end()) {
		return std::string_view{};
	}
	return std::string_view{range.first, range.second};
}
