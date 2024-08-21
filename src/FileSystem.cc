#include <m2/FileSystem.h>
#include <sstream>

m2::expected<std::string> m2::read_file(const std::filesystem::path& path) {
	FILE* file = fopen(path.string().c_str(), "r");
	m2_return_unexpected_message_unless(file, "Unable to open file " + path.string());

	std::stringstream ss;
	while (not feof(file)) {
		char buffer[512];
		ss.write(buffer, static_cast<long>(fread(buffer, 1, 512, file)));
	}

	fclose(file);
	return ss.str();
}

m2::void_expected m2::write_to_file(const std::string& str, const std::filesystem::path& path) {
	FILE* file = fopen(path.string().c_str(), "w");
	m2_return_unexpected_message_unless(file, "Unable to open file " + path.string());
	auto size = str.size();
	bool success = (fwrite(str.data(), 1, size, file) == size);
	fclose(file);
	if (success) {
		return {};
	} else {
		return make_unexpected("Unable to write string of size " + std::to_string(size));
	}
}

std::vector<std::filesystem::path> m2::list_files(const std::filesystem::path& dir, const std::string& ends_with) {
	std::vector<std::filesystem::path> files;
	// Iterate over files in the directory
	for (const auto& dir_entry : std::filesystem::directory_iterator{dir}) {
		// List only regular files
		if (dir_entry.is_regular_file()) {
			const auto &path = dir_entry.path();
			// Check suffix
			if (ends_with.empty() || path.string().ends_with(ends_with)) {
				files.emplace_back(path);
			}
		}
	}
	std::sort(files.begin(), files.end());
	return files;
}
