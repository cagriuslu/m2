#pragma once
#include "Meta.h"
#include <filesystem>

namespace m2 {
	std::filesystem::path resource_path();

    expected<std::string> read_file(const std::filesystem::path& path);
    void_expected write_to_file(const std::string& str, const std::filesystem::path& path);
    std::vector<std::filesystem::path> list_files(const std::filesystem::path& dir, const std::string& ends_with = "");
}
