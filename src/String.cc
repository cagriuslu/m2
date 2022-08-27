#include <m2/String.h>
#include <sstream>

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

m2::Value<std::string> m2::string::read_file(const std::string& path) {
	FILE* file = fopen(path.c_str(), "r");
	m2_fail_unless(file, "Unable to open file " + path);

	std::stringstream ss;
	while (not feof(file)) {
		char buffer[512];
		ss.write(buffer, static_cast<long>(fread(buffer, 1, 512, file)));
	}

	fclose(file);
	return ss.str();
}

m2::VoidValue m2::string::write_to_file(const std::string& str, const std::string& path) {
	FILE* file = fopen(path.c_str(), "w");
	m2_fail_unless(file, "Unable to open file " + path);
	auto size = str.size();
	bool success = (fwrite(str.data(), 1, size, file) == size);
	fclose(file);
	if (success) {
		return {};
	} else {
		return failure("Unable to write string of size " + std::to_string(size));
	}
}
