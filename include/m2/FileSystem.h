#ifndef M2_FILESYSTEM_H
#define M2_FILESYSTEM_H

#include "Value.h"
#include <filesystem>

namespace m2 {
	Value<std::string> read_file(const std::string& path);
	VoidValue write_to_file(const std::string& str, const std::string& path);
	std::vector<std::filesystem::path> list_files(const std::filesystem::path& dir, const std::string& ends_with = "");
}

#endif //M2_FILESYSTEM_H
