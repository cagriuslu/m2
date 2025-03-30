#pragma once
#include "Meta.h"
#include <filesystem>

namespace m2 {
	std::filesystem::path ResourcePath();

    expected<std::string> ReadFile(const std::filesystem::path& path);
    void_expected WriteToFile(const std::string& str, const std::filesystem::path& path);
    std::vector<std::filesystem::path> ListFiles(const std::filesystem::path& dir, const std::string& ends_with = "");
}
