#pragma once
#include "Value.h"
#include <filesystem>

namespace m2 {
    Value<std::string> read_file(const std::filesystem::path& path);
    VoidValue write_to_file(const std::string& str, const std::filesystem::path& path);
    std::vector<std::filesystem::path> list_files(const std::filesystem::path& dir, const std::string& ends_with = "");
}
