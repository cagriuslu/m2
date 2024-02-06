#include <m2/Log.h>
#include <m2/detail/ArgumentParser.h>

std::vector<std::string> m2::to_argument_list(int argc, char** argv) {
	std::vector<std::string> list;
	for (int i = 1; i < argc; ++i) {
		std::string arg{argv[i]};
		LOG_INFO("Argument: ", arg);
		list.emplace_back(std::move(arg));
	}
	return list;
}

std::optional<std::string> m2::parse_argument(
    const std::vector<std::string>& argument_list, const std::string& raw_argument_key) {
	std::string arg_flag = "--" + raw_argument_key;  // Ex. --silent
	std::string arg_key = "--" + raw_argument_key + "=";  // Ex. --slowdown=2
	for (const auto& arg : argument_list) {
		if (arg == arg_flag) {
			return "";  // Optional becomes true
		} else if (arg.starts_with(arg_key)) {
			auto raw_value = arg.substr(arg_key.size());  // Extract the string after '='
			if (2 <= raw_value.size()) {
				if ((raw_value.front() == '"' && raw_value.back() == '"') ||
				    (raw_value.front() == '\'' && raw_value.back() == '\'')) {
					return raw_value.substr(1, raw_value.size() - 2);
				} else {
					return raw_value;
				}
			} else {
				return raw_value;
			}
		}
	}
	return std::nullopt;
}
